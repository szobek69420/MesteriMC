#define _CRT_SECURE_NO_WARNINGS
#include "canvas.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../font_handler/font_handler.h"
#include "../text/text_renderer.h"

#include "../button/button_renderer.h"

#include "../image_renderer/image_renderer.h"

#include "../../utils/seqtor.h"

#include "../../window/window.h"

#define CANVAS_FONT_SIZE 72

#define CANVAS_COMPONENT_TEXT 0
#define CANVAS_COMPONENT_BUTTON 1
#define CANVAS_COMPONENT_IMAGE 2
#define CANVAS_COMPONENT_SLIDER 3

static int componentIDCounter = 0;//increases by one if a component has been added to the canvas

struct canvasText {
	char* text;
	float r, g, b;//white is rgb(255,255,255)
	float scale;//for the textRenderer
};
typedef struct canvasText canvasText;

struct canvasButton {
	canvasText ct;
	float textWidth, textHeight;
	float borderR, borderG, borderB;
	float fillR, fillG, fillB;
	float borderWidth, borderRadius;
	int transparentBackground;
	void (*clicked)(void*);
	void* clickedParam;
};
typedef struct canvasButton canvasButton;

struct canvasImage {
	unsigned int textureId;
	float uvX, uvY, uvWidth, uvHeight;
	float tintR, tintG, tintB;
	void (*clicked)(void*);
	void* clickedParam;
};
typedef struct canvasImage canvasImage;

struct canvasSlider {
	float value, prevValue, min, max;
	int wholeNumbers;

	float borderR, borderG, borderB;
	float fillR, fillG, fillB;
	float borderWidth, borderRadius;
	int transparentBackground;

	float knobWidth, knobHeight;
	float knobBorderR, knobBorderG, knobBorderB;
	float knobFillR, knobFillG, knobFillB;
	float knobBorderWidth, knobBorderRadius;
	int knobTransparentBackground;

	void (*dragged)(float);
};
typedef struct canvasSlider canvasSlider;

struct canvasComponent {
	int id;
	int componentType;
	int hAlign, vAlign;
	int x, y;//the position of the (0,0) point depends on the alignment (for example for a combo of CANVAS_ALIGN_LEFT and CANVAS_ALIGN_BOTTOM it is the bottom left corner of the screen, but for CANVAS_ALIGN_RIGHT and CANVAS_ALIGN_TOP it is the top right corner of the screen)
	float originX, originY;//(0,0) is the bottom left corner (for example CANVAS_ALIGN_LEFT and CANVAS_ALIGN_TOP for (x,y)=(10,0) equals an (originX,originY)=(10,canvas_height-component_height)
	float width, height;
	int inDrag;//ha az eger elhagyna a komponens teruletet, de nem eresztene fel a gombot a felhasznalo, akkor az meg drag legyen

	union componentData {
		canvasText ct;
		canvasButton cb;
		canvasImage ci;
		canvasSlider cs;
	};
};
typedef struct canvasComponent canvasComponent;

struct canvas {
	int width, height;

	font f;
	textRenderer tr;

	buttonRenderer* br;

	imageRenderer* ir;

	seqtor_of(canvasComponent) components;
};

void canvas_calculatePositions(canvas* c);
void canvas_destroyComponent(canvasComponent* cc);

//font handler should be initialized before calling this
canvas* canvas_create(int width, int height, const char* fontSauce)
{
	canvas* c=malloc(sizeof(canvas));
	c->width = width;
	c->height = height;

	if(fontSauce!=NULL)
		c->f = fontHandler_loadFont(fontSauce, CANVAS_FONT_SIZE);

	c->tr = textRenderer_create(width, height);

	c->br = buttonRenderer_create(width, height);

	c->ir = imageRenderer_create(width, height);

	seqtor_init(c->components, 1);

	return c;
}

void canvas_destroy(canvas* c)
{
	fontHandler_destroyFont(&c->f);
	textRenderer_destroy(&c->tr);
	buttonRenderer_destroy(c->br);
	imageRenderer_destroy(c->ir);

	while (seqtor_size(c->components) > 0)
	{
		canvas_destroyComponent(&seqtor_back(c->components));
		seqtor_pop_back(c->components);
	}
	seqtor_destroy(c->components);

	free(c);
}

canvasComponent* canvas_getComponent(canvas* c, int id)
{
	canvasComponent* cc = NULL;
	for (int i = 0; i < c->components.size; i++)
	{
		if (id == seqtor_at(c->components, i).id)
		{
			cc = &seqtor_at(c->components, i).id;
			break;
		}
	}

	return cc;
}

void canvas_setSize(canvas* c, int width, int height)
{
	c->width = width;
	c->height = height;

	textRenderer_setSize(&c->tr, width, height);
	buttonRenderer_setSize(c->br, width, height);
	imageRenderer_setSize(c->ir, width, height);

	canvas_calculatePositions(c);
}

void canvas_render(canvas* c, int mouseX, int mouseY, int mousePressed)
{
	mouseY = window_getHeight() - mouseY;//mert a glfw (0,0) koordinataja a bal felso sarokban van, mig a canvase a bal alsoban
	canvasComponent* cc;
	int isInBounds;
	for (int i = 0; i < seqtor_size(c->components); i++)
	{
		cc = &seqtor_at(c->components,i);

		isInBounds = 1;
		if (cc->originX > mouseX || cc->originX + cc->width<mouseX || cc->originY>mouseY || cc->originY + cc->height < mouseY)
			isInBounds = 0;

		switch (cc->componentType)
		{
		case CANVAS_COMPONENT_TEXT:
			if (cc->ct.text == NULL || strlen(cc->ct.text) == 0)
				break;

			textRenderer_setColour(&c->tr, cc->ct.r, cc->ct.g, cc->ct.b);
			textRenderer_render(&c->tr, &c->f, cc->ct.text, cc->originX, cc->originY, cc->ct.scale);
			break;

		case CANVAS_COMPONENT_BUTTON:
			buttonRenderer_setBackgroundTransparency(c->br, cc->cb.transparentBackground);
			if (cc->inDrag)
			{
				buttonRenderer_setFillColour(c->br, 0.5f * cc->cb.fillR, 0.5f * cc->cb.fillG, 0.5f * cc->cb.fillB);
				buttonRenderer_setBorderColour(c->br, 0.5f * cc->cb.borderR, 0.5f * cc->cb.borderG, 0.5f * cc->cb.borderB);
			}
			else if (isInBounds&&cc->cb.clicked!=NULL)
			{
				buttonRenderer_setFillColour(c->br, 0.8f * cc->cb.fillR, 0.8f * cc->cb.fillG, 0.8f * cc->cb.fillB);
				buttonRenderer_setBorderColour(c->br, 0.8f * cc->cb.borderR, 0.8f * cc->cb.borderG, 0.8f * cc->cb.borderB);
			}
			else
			{
				buttonRenderer_setFillColour(c->br, cc->cb.fillR, cc->cb.fillG, cc->cb.fillB);
				buttonRenderer_setBorderColour(c->br, cc->cb.borderR, cc->cb.borderG, cc->cb.borderB);
			}
			buttonRenderer_render(c->br, cc->originX, cc->originY, cc->width, cc->height, cc->cb.borderWidth, cc->cb.borderRadius);
			if (cc->cb.ct.text != NULL)
			{
				textRenderer_setColour(&c->tr, cc->cb.ct.r, cc->cb.ct.g, cc->cb.ct.b);
				textRenderer_render(
					&c->tr, 
					&c->f, 
					cc->cb.ct.text, 
					cc->originX+0.5f*cc->width-0.5f*cc->cb.textWidth, 
					cc->originY + 0.5f * cc->height - 0.5f * cc->cb.textHeight,
					cc->ct.scale);
			}
			break;

		case CANVAS_COMPONENT_IMAGE:
			if (cc->ci.textureId != 0)
			{
				if(cc->inDrag)
					imageRenderer_setTint(c->ir, 0.5f * cc->ci.tintR, 0.5f * cc->ci.tintG, 0.5f * cc->ci.tintB);
				else if (isInBounds && cc->ci.clicked != NULL)
					imageRenderer_setTint(c->ir, 0.8f * cc->ci.tintR, 0.8f * cc->ci.tintG, 0.8f * cc->ci.tintB);
				else
					imageRenderer_setTint(c->ir, cc->ci.tintR, cc->ci.tintG, cc->ci.tintB);
				imageRenderer_render(
					c->ir,
					cc->ci.textureId,
					cc->originX,
					cc->originY,
					cc->width,
					cc->height,
					cc->ci.uvX,
					cc->ci.uvY,
					cc->ci.uvWidth,
					cc->ci.uvHeight);
			}
			break;

		case CANVAS_COMPONENT_SLIDER:
			//background
			buttonRenderer_setBackgroundTransparency(c->br, cc->cs.transparentBackground);
			buttonRenderer_setFillColour(c->br, cc->cs.fillR, cc->cs.fillG, cc->cs.fillB);
			buttonRenderer_setBorderColour(c->br, cc->cs.borderR, cc->cs.borderG, cc->cs.borderB);
			buttonRenderer_render(c->br, cc->originX, cc->originY, cc->width, cc->height, cc->cs.borderWidth, cc->cs.borderRadius);
			//knob
			float knobX = ((cc->cs.value - cc->cs.min) / (cc->cs.max - cc->cs.min)) * cc->width + cc->originX - 0.5f * cc->cs.knobWidth;
			float knobY = 0.5f * cc->height - 0.5f * cc->cs.knobHeight + cc->originY;

			buttonRenderer_setBackgroundTransparency(c->br, cc->cs.knobTransparentBackground);
			if (cc->inDrag)
			{
				buttonRenderer_setFillColour(c->br, 0.8f * cc->cs.knobFillR, 0.8f * cc->cs.knobFillG, 0.8f * cc->cs.knobFillB);
				buttonRenderer_setBorderColour(c->br, 0.8f * cc->cs.knobBorderR, 0.8f * cc->cs.knobBorderG, 0.8f * cc->cs.knobBorderB);
			}
			else
			{
				buttonRenderer_setFillColour(c->br, cc->cs.knobFillR, cc->cs.knobFillG, cc->cs.knobFillB);
				buttonRenderer_setBorderColour(c->br, cc->cs.knobBorderR, cc->cs.knobBorderG, cc->cs.knobBorderB);
			}
			buttonRenderer_render(c->br, knobX, knobY, cc->cs.knobWidth, cc->cs.knobHeight, cc->cs.knobBorderWidth, cc->cs.knobBorderRadius);
			break;
		}
	}
}

void canvas_checkMouseInput(canvas* c, int mouseX, int mouseY, int mouseDown, int mousePressed, int mouseClicked)
{
	mouseY = window_getHeight() - mouseY;
	canvasComponent* cc;
	int clickCallbackExecuted = 0;
	int inBounds = 0;
	for (int i = seqtor_size(c->components)-1; i >=0; i--)
	{
		cc = &seqtor_at(c->components, i);

		inBounds = 69;
		if (cc->originX > mouseX || cc->originX + cc->width<mouseX || cc->originY>mouseY || cc->originY + cc->height < mouseY)
			inBounds = 0;


		switch (cc->componentType)
		{

		case CANVAS_COMPONENT_BUTTON:
			if (mouseClicked)
			{
				if (cc->inDrag && inBounds&& cc->cb.clicked != NULL && clickCallbackExecuted == 0)
				{
					cc->cb.clicked(cc->cb.clickedParam);
					clickCallbackExecuted = 69;
				}
				cc->inDrag = 0;
			}
			if (mousePressed)
			{
				if (inBounds && cc->cb.clicked != NULL)
					cc->inDrag = 69;
			}
			break;

		case CANVAS_COMPONENT_IMAGE:
			if (mouseClicked)
			{
				if (cc->inDrag && inBounds && cc->ci.clicked != NULL && clickCallbackExecuted == 0)
				{
					cc->ci.clicked(cc->ci.clickedParam);
					clickCallbackExecuted = 69;
				}
				cc->inDrag = 0;
			}
			if (mousePressed)
			{
				if (inBounds && cc->ci.clicked != NULL)
					cc->inDrag = 69;
			}
			break;

		case CANVAS_COMPONENT_SLIDER:
			if (mousePressed&&cc->cs.dragged!=NULL)
			{
				if (inBounds)
				{
					float value = (mouseX - cc->originX) / cc->width;
					if (value < 0)
						value = 0;
					if (value > 1)
						value = 1;
					value = value * (cc->cs.max - cc->cs.min) + cc->cs.min;
					if(cc->cs.wholeNumbers)
						cc->cs.value = lroundf(value);
					else
						cc->cs.value = value;


					if (fabsf(cc->cs.value - cc->cs.prevValue) > 0.01f)
					{
						cc->cs.dragged(cc->cs.value);
						cc->cs.prevValue = cc->cs.value;
					}


					cc->inDrag = 69;
				}
			}
			else if (mouseDown && cc->inDrag)
			{
				float value = (mouseX - cc->originX) / cc->width;
				if (value < 0)
					value = 0;
				if (value > 1)
					value = 1;
				value = value * (cc->cs.max - cc->cs.min) + cc->cs.min;
				if (cc->cs.wholeNumbers)
					cc->cs.value = lroundf(value);
				else
					cc->cs.value = value;

				if (fabsf(cc->cs.value - cc->cs.prevValue) > 0.01f)
				{
					cc->cs.dragged(cc->cs.value);
					cc->cs.prevValue = cc->cs.value;
				}
			}
			else
				cc->inDrag = 0;

			break;

		default:
			continue;
		}
	}
}

void canvas_calculatePosition(canvas* c, canvasComponent* cc)
{
	//horizontal align
	switch (cc->hAlign)
	{
	case CANVAS_ALIGN_LEFT: //x is the horizontal distance between the left side of the screen and the left side of the component (positive direction is to the right)
		cc->originX = cc->x;
		break;

	case CANVAS_ALIGN_CENTER://x is the horizontal distance between the center of the screen and the center of the component (positive direction is to the right)
		cc->originX = 0.5f * c->width + cc->x - 0.5f * cc->width;
		break;

	case CANVAS_ALIGN_RIGHT://x is the horizontal distance between the right side of the screen and the right side of the component (positive direction is to the left)
		cc->originX = c->width - cc->x - cc->width;
		break;
	}

	//vertical align
	switch (cc->vAlign)
	{
	case CANVAS_ALIGN_BOTTOM://y is the vertical distance between the bottom side of the screen and the bottom side of the component (positive direction is up)
		cc->originY = cc->y;
		break;

	case CANVAS_ALIGN_MIDDLE://y is the vertical distance between the center of the screen and the center of the component (positive direction is up)
		cc->originY = 0.5f * c->height + cc->y - 0.5f * cc->height;
		break;

	case CANVAS_ALIGN_TOP://y is the vertical distance between the top side of the screen and the top side of the component (positive direction is down)
		cc->originY = c->height - cc->y - cc->height;
		break;
	}
}

void canvas_calculatePositions(canvas* c)
{
	canvasComponent* cc;
	for (int i = 0; i < seqtor_size(c->components); i++)
	{
		cc = &seqtor_at(c->components, i);

		canvas_calculatePosition(c, cc);
	}
}


void canvas_removeComponent(canvas* c, int id)
{
	for (int i = 0; i < c->components.size; i++)
	{
		if (id == seqtor_at(c->components, i).id)
		{
			canvas_destroyComponent(&seqtor_at(c->components, i));
			seqtor_remove_at(c->components, i);
			break;
		}
	}
}

void canvas_destroyComponent(canvasComponent* cc)
{
	switch (cc->componentType)
	{
	case CANVAS_COMPONENT_TEXT:
		if(cc->ct.text!=NULL)
			free(cc->ct.text);
		cc->ct.text = NULL;
		break;

	case CANVAS_COMPONENT_BUTTON:
		if (cc->cb.ct.text != NULL)
			free(cc->cb.ct.text);
		cc->cb.ct.text = NULL;
		break;

	case CANVAS_COMPONENT_IMAGE:
		//nothing
		break;
	}
}

//GENERAL------------------------------------------------------------------------------------------------------------------------

void canvas_setComponentPosition(canvas* c, int id, int x, int y)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL)
		return;

	cc->x = x;
	cc->y = y;

	canvas_calculatePosition(c, cc);
}

void canvas_setComponentAlignment(canvas* c, int id, int hAlign, int vAlign)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL)
		return;

	cc->hAlign = hAlign;
	cc->vAlign = vAlign;

	canvas_calculatePosition(c, cc);
}

void canvas_setComponentSize(canvas* c, int id, float width, float height)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL||cc->componentType==CANVAS_COMPONENT_TEXT)
		return;

	cc->width = width;
	cc->height = height;

	canvas_calculatePosition(c, cc);
}

//TEXT---------------------------------------------------------------------------------------------------------------------------

int canvas_addText(canvas* c, const char* text, int hAlign, int vAlign, int x, int y, float r, float g, float b, int fontSize)
{
	canvasComponent cc;
	cc.componentType = CANVAS_COMPONENT_TEXT;
	cc.id = componentIDCounter++;
	cc.x = x;
	cc.y = y;
	cc.hAlign = hAlign;
	cc.vAlign = vAlign;

	cc.inDrag = 0;

	cc.ct.text = malloc((strlen(text) + 1) * sizeof(char));
	strcpy(cc.ct.text, text);

	cc.ct.r = r;
	cc.ct.g = g;
	cc.ct.b = b;

	cc.ct.scale = (float)fontSize / CANVAS_FONT_SIZE;
	cc.height = cc.ct.scale * c->f.lineHeight;
	cc.width = cc.ct.scale * fontHandler_calculateTextLength(&c->f, cc.ct.text);

	seqtor_push_back(c->components, cc);

	canvas_calculatePosition(c, &seqtor_back(c->components));

	return cc.id;
}

void canvas_setTextText(canvas* c, int id, const char* text)
{
	canvasComponent* cc;

	cc = canvas_getComponent(c, id);

	if (cc == NULL||cc->componentType!=CANVAS_COMPONENT_TEXT)
		return;

	free(cc->ct.text);
	cc->ct.text = malloc((strlen(text) + 1) * sizeof(char));
	strcpy(cc->ct.text, text);

	cc->width = cc->ct.scale * fontHandler_calculateTextLength(&c->f, cc->ct.text);

	canvas_calculatePosition(c, cc);
}

void canvas_setTextColour(canvas* c, int id, float r, float g, float b)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_TEXT)
		return;

	cc->ct.r = r;
	cc->ct.g = g;
	cc->ct.b = b;
}

void canvas_setTextFontSize(canvas* c, int id, int fontSize)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_TEXT)
		return;

	cc->ct.scale = (float)fontSize / CANVAS_FONT_SIZE;
	cc->height = cc->ct.scale * c->f.lineHeight;
	cc->width = cc->ct.scale * fontHandler_calculateTextLength(&c->f, cc->ct.text);

	canvas_calculatePosition(c, cc);
}

//BUTTON---------------------------------------------------------------------------------------------------------------------------

int canvas_addButton(canvas* c, int hAlign, int vAlign, int x, int y, float width, float height)
{
	canvasComponent cc;
	cc.componentType = CANVAS_COMPONENT_BUTTON;
	cc.id = componentIDCounter++;
	cc.x = x;
	cc.y = y;
	cc.hAlign = hAlign;
	cc.vAlign = vAlign;
	cc.width = width;
	cc.height = height;
	cc.inDrag = 0;

	cc.cb.ct.text = NULL;
	cc.cb.ct.r = 0;		cc.cb.ct.g = 0;		cc.cb.ct.b = 0;

	cc.cb.clicked = NULL;
	cc.cb.fillR = 1;		cc.cb.fillG = 1;		cc.cb.fillB = 1;
	cc.cb.borderR = 0.8f;	cc.cb.borderG = 0.8f;	cc.cb.borderB = 0.8f;

	cc.cb.borderWidth = 10;	cc.cb.borderRadius = 20;
	cc.cb.transparentBackground = 0;

	seqtor_push_back(c->components, cc);

	canvas_calculatePosition(c, &seqtor_back(c->components));

	return cc.id;
}

void canvas_setButtonFillColour(canvas* c, int id, float r, float g, float b)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_BUTTON)
		return;

	cc->cb.fillR = r;	cc->cb.fillG = g;	cc->cb.fillB = b;
}

void canvas_setButtonBorderColour(canvas* c, int id, float r, float g, float b)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_BUTTON)
		return;

	cc->cb.borderR = r;		cc->cb.borderG = g;		cc->cb.borderB = b;
}

void canvas_setButtonBackgroundTransparency(canvas* c, int id, int transparentBackground)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_BUTTON)
		return;

	cc->cb.transparentBackground = transparentBackground;
}

void canvas_setButtonBorder(canvas* c, int id, float borderWidth, float borderRadius)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_BUTTON)
		return;

	cc->cb.borderRadius = borderRadius;
	cc->cb.borderWidth = borderWidth;
}

void canvas_setButtonClicked(canvas* c, int id, void (*onClick)(void*), void* param)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_BUTTON)
		return;

	cc->cb.clicked = onClick;
	cc->cb.clickedParam = param;
}

void canvas_setButtonText(canvas* c, int id, const char* text, int fontSize, float r, float g, float b)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_BUTTON)
		return;

	if (cc->cb.ct.text != NULL)
		free(cc->cb.ct.text);

	if (text == NULL || strlen(text) == 0)
	{
		cc->cb.ct.text = NULL;
		return;
	}

	cc->cb.ct.text = malloc((strlen(text) + 1) * sizeof(char));
	strcpy(cc->cb.ct.text, text);

	cc->cb.ct.scale = (float)fontSize / CANVAS_FONT_SIZE;
	cc->cb.textHeight = cc->cb.ct.scale * c->f.lineHeight;
	cc->cb.textWidth = cc->cb.ct.scale * fontHandler_calculateTextLength(&c->f, cc->cb.ct.text);

	cc->cb.ct.r = r;
	cc->cb.ct.g = g;
	cc->cb.ct.b = b;
}

//IMAGE----------------------------------------------------------------------------------------------

int canvas_addImage(canvas* c, int hAlign, int vAlign, int x, int y, float width, float height, unsigned int textureId)
{
	canvasComponent cc;
	cc.componentType = CANVAS_COMPONENT_IMAGE;
	cc.id = componentIDCounter++;
	cc.x = x;
	cc.y = y;
	cc.hAlign = hAlign;
	cc.vAlign = vAlign;
	cc.width = width;
	cc.height = height;
	cc.inDrag = 0;

	cc.ci.textureId = textureId;
	cc.ci.tintR = 1;	cc.ci.tintG = 1;	cc.ci.tintB = 1;
	cc.ci.uvX = 0;	cc.ci.uvY = 0;	cc.ci.uvWidth = 1;	cc.ci.uvHeight = 1;

	cc.ci.clicked = NULL;
	cc.ci.clickedParam = NULL;

	seqtor_push_back(c->components, cc);

	canvas_calculatePosition(c, &seqtor_back(c->components));

	return cc.id;
}

void canvas_setImageTint(canvas* c, int id, float r, float g, float b)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_IMAGE)
		return;

	cc->ci.tintR = r;
	cc->ci.tintG = g;
	cc->ci.tintB = b;
}

void canvas_setImageTexture(canvas* c, int id, unsigned int textureId)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_IMAGE)
		return;

	cc->ci.textureId = textureId;
}

void canvas_setImageUV(canvas* c, int id, float uvX, float uvY, float uvWidth, float uvHeight)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_IMAGE)
		return;

	cc->ci.uvX = uvX;
	cc->ci.uvY = uvY;
	cc->ci.uvWidth = uvWidth;
	cc->ci.uvHeight = uvHeight;
}

void canvas_setImageClicked(canvas* c, int id, void (*onClick)(void*), void* param)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_IMAGE)
		return;

	cc->ci.clicked = onClick;
	cc->ci.clickedParam = param;
}

//SLIDER----------------------------------------------------------------------------------------------

int canvas_addSlider(canvas* c, int hAlign, int vAlign, int x, int y, float width, float height, float knobWidth, float knobHeight, int wholeNumbers)
{
	canvasComponent cc;
	cc.componentType = CANVAS_COMPONENT_SLIDER;
	cc.id = componentIDCounter++;
	cc.x = x;
	cc.y = y;
	cc.hAlign = hAlign;
	cc.vAlign = vAlign;
	cc.width = width;
	cc.height = height;
	cc.inDrag = 0;

	cc.cs.min = 0;
	cc.cs.max = 1;
	cc.cs.value = 0.5f;
	cc.cs.prevValue = 0.5f;
	cc.cs.wholeNumbers = wholeNumbers;


	cc.cs.dragged = NULL;

	cc.cs.fillR = 1;		cc.cs.fillG = 1;		cc.cs.fillB = 1;
	cc.cs.borderR = 0.8f;	cc.cs.borderG = 0.8f;	cc.cs.borderB = 0.8f;
	cc.cs.borderWidth = 10;	cc.cs.borderRadius = 20;
	cc.cs.transparentBackground = 0;

	cc.cs.knobWidth = knobWidth;
	cc.cs.knobHeight = knobHeight;
	cc.cs.knobFillR = 1;	cc.cs.knobFillG = 1;	cc.cs.knobFillB = 1;
	cc.cs.knobBorderR = 0.8f;	cc.cs.knobBorderG = 0.8f;	cc.cs.knobBorderB = 0.8f;
	cc.cs.knobBorderWidth = 10;	cc.cs.knobBorderRadius = 20;
	cc.cs.knobTransparentBackground = 0;

	seqtor_push_back(c->components, cc);

	canvas_calculatePosition(c, &seqtor_back(c->components));

	return cc.id;
}

void canvas_setSliderCallback(canvas* c, int id, void(*dragged)(float))
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.dragged = dragged;
}

void canvas_setSliderValue(canvas* c, int id, float value)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.prevValue = value;
	cc->cs.value=value;
}

void canvas_setSliderBounds(canvas* c, int id, float minValue, float maxValue)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.min = minValue;
	cc->cs.max = maxValue;
}

void canvas_setSliderWholeNumbers(canvas* c, int id, int wholeNumbers)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.wholeNumbers = wholeNumbers;
}

void canvas_setSliderBackgroundTransparency(canvas* c, int id, int transparentBackground)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.transparentBackground = transparentBackground;
}

void canvas_setSliderBackgroundBorder(canvas* c, int id, float borderWidth, float borderRadius)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.borderRadius = borderRadius;
	cc->cs.borderWidth = borderWidth;
}

void canvas_setSliderBackgroundFillColour(canvas* c, int id, float r, float g, float b)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.fillR = r;	cc->cs.fillG = g;	cc->cs.fillB = b;
}

void canvas_setSliderBackgroundBorderColour(canvas* c, int id, float r, float g, float b)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.borderR = r;		cc->cs.borderG = g;		cc->cs.borderB = b;
}

void canvas_setSliderKnobTransparency(canvas* c, int id, int knobTransparentBackground)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.knobTransparentBackground = knobTransparentBackground;
}

void canvas_setSliderKnobBorder(canvas* c, int id, float borderWidth, float borderRadius)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.knobBorderRadius = borderRadius;
	cc->cs.knobBorderWidth = borderWidth;
}

void canvas_setSliderKnobFillColour(canvas* c, int id, float r, float g, float b)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.knobFillR = r;	cc->cs.knobFillG = g;	cc->cs.knobFillB = b;
}

void canvas_setSliderKnobBorderColour(canvas* c, int id, float r, float g, float b)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.knobBorderR = r;		cc->cs.knobBorderG = g;		cc->cs.knobBorderB = b;
}

void canvas_setKnobWidth(canvas* c, int id, float knobWidth, float knobHeight)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL || cc->componentType != CANVAS_COMPONENT_SLIDER)
		return;

	cc->cs.knobWidth=knobWidth;		cc->cs.knobHeight=knobHeight;
}

