#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "../../shader/shader.h"
#include "../font_handler/font_handler.h"

//egy projekcios matrix helyezi el a betuket a kepernyon, mely nincs elmentve, csupan a shaderbe betoltve
//a szin szinten csak a shaderben van tarolva
typedef struct {
	unsigned int vao;
	unsigned int vbo;
	unsigned int ebo;

	shader program;
} textRenderer;

textRenderer textRenderer_create(int width, int height);//width and height in pixels
void textRenderer_destroy(textRenderer* renderer);

void textRenderer_render(textRenderer* renderer, font* f, const char* text, float x, float y, float scale);

void textRenderer_setColour(textRenderer* renderer, float r, float g, float b);
void textRenderer_setSize(textRenderer* renderer, int width, int height);
#endif