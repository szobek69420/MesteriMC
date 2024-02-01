#define _CRT_SECURE_NO_WARNINGS
#include "canvas.h"

#include <stdlib.h>
#include <string.h>

#include "../font_handler/font_handler.h"
#include "../text/text_renderer.h"

#include "../../utils/seqtor.h"

#define CANVAS_FONT_SIZE 48

#define CANVAS_COMPONENT_TEXT 0
#define CANVAS_COMPONENT_BUTTON 1

static int componentIDCounter = 0;//increases by one if a component has been added to the canvas

struct canvasText {
	char* text;
	float r, g, b;//white is rgb(255,255,255)
	float scale;//for the textRenderer
};
typedef struct canvasText canvasText;

struct canvasButton {
	canvasText ct;
	float normalR, normalG, normalB;
	float hoverR, hoverG, hoverB;
	void (*clicked)(void*);
};
typedef struct canvasButton canvasButton;

struct canvasComponent {
	int id;
	int componentType;
	int hAlign, vAlign;
	int x, y;//the position of the (0,0) point depends on the alignment (for example for a combo of CANVAS_ALIGN_LEFT and CANVAS_ALIGN_BOTTOM it is the bottom left corner of the screen, but for CANVAS_ALIGN_RIGHT and CANVAS_ALIGN_TOP it is the top right corner of the screen)
	float originX, originY;//(0,0) is the bottom left corner (for example CANVAS_ALIGN_LEFT and CANVAS_ALIGN_TOP for (x,y)=(10,0) equals an (originX,originY)=(10,canvas_height-component_height)
	float width, height;

	union componentData {
		canvasText ct;
		canvasButton cb;
	};
};
typedef struct canvasComponent canvasComponent;

struct canvas {
	int width, height;

	font f;
	textRenderer tr;

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

	c->f = fontHandler_loadFont(fontSauce,CANVAS_FONT_SIZE);

	c->tr = textRenderer_create(width, height);

	seqtor_init(c->components, 1);

	return c;
}

void canvas_destroy(canvas* c)
{
	fontHandler_destroyFont(&c->f);
	textRenderer_destroy(&c->tr);

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

	canvas_calculatePositions(c);
}

void canvas_render(canvas* c)
{
	canvasComponent* cc;
	int x, y, width, height;
	for (int i = 0; i < seqtor_size(c->components); i++)
	{
		cc = &seqtor_at(c->components,i);

		switch (cc->componentType)
		{
		case CANVAS_COMPONENT_TEXT:
			textRenderer_setColour(&c->tr, cc->ct.r, cc->ct.g, cc->ct.b);
			textRenderer_render(&c->tr, &c->f, cc->ct.text, cc->originX, cc->originY, cc->ct.scale);
			break;
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

int canvas_addText(canvas* c, const char* text, int hAlign, int vAlign, int x, int y,  float r, float g, float b, int fontSize)
{
	canvasComponent cc;
	cc.componentType = CANVAS_COMPONENT_TEXT;
	cc.id = componentIDCounter++;
	cc.x = x;
	cc.y = y;
	cc.hAlign = hAlign;
	cc.vAlign = vAlign;

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

void canvas_setTextText(canvas* c, int id, const char* text)
{
	canvasComponent* cc;

	cc = canvas_getComponent(c, id);

	if (cc == NULL)
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
	if (cc == NULL)
		return;

	cc->ct.r = r;
	cc->ct.g = g;
	cc->ct.b = b;
}

void canvas_setTextFontSize(canvas* c, int id, int fontSize)
{
	canvasComponent* cc;
	cc = canvas_getComponent(c, id);
	if (cc == NULL)
		return;

	cc->ct.scale = (float)fontSize / CANVAS_FONT_SIZE;
	cc->height = cc->ct.scale * c->f.lineHeight;
	cc->width = cc->ct.scale * fontHandler_calculateTextLength(&c->f, cc->ct.text);

	canvas_calculatePosition(c, cc);
}

void canvas_destroyComponent(canvasComponent* cc)
{
	switch (cc->componentType)
	{
	case CANVAS_COMPONENT_TEXT:
		free(cc->ct.text);
		break;
	}
}

