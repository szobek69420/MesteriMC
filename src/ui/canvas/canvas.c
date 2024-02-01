#define _CRT_SECURE_NO_WARNINGS
#include "canvas.h"

#include <stdlib.h>
#include <string.h>

#include "../font_handler/font_handler.h"
#include "../text/text_renderer.h"

#include "../../utils/seqtor.h"

#define CANVAS_FONT_SIZE 48

#define CANVAS_COMPONENT_TEXT 0

static int componentIDCounter = 0;

struct canvasText {
	char* text;
	float r, g, b;
	int fontSize;
};

struct canvasComponent {
	int id;
	int componentType;
	int hAlign, vAlign;
	int x, y;

	union componentData {
		canvasText ct;
	};
};
typedef struct canvasComponent canvasComponent;

struct canvas {
	int width, height;

	font f;
	textRenderer tr;

	seqtor_of(canvasComponent) components;
};

void canvas_destroyComponent(canvasComponent* cc);

//font handler should be initialized before calling this
canvas canvas_create(int width, int height, const char* fontSauce)
{
	canvas c;
	c.width = width;
	c.height = height;

	c.f = fontHandler_loadFont(fontSauce,CANVAS_FONT_SIZE);

	c.tr = textRenderer_create(width, height);

	seqtor_init(c.components, 1);
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
}

int canvas_addText(canvas* c, const char* text, int hAlign, int vAlign, int x, int y,  float r, float g, float b)
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

	seqtor_push_back(c->components, cc);

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

void canvas_destroyComponent(canvasComponent* cc)
{
	switch (cc->componentType)
	{
	case CANVAS_COMPONENT_TEXT:
		free(cc->ct.text);
		break;
	}
}
