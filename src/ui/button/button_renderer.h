#ifndef BUTTON_RENDERER_H
#define BUTTON_RENDERER_H

struct buttonRenderer;
typedef struct buttonRenderer buttonRenderer;

buttonRenderer* buttonRenderer_create(int width, int height);
void buttonRenderer_destroy(buttonRenderer* br);

void buttonRenderer_render(buttonRenderer* br, int x, int y, int width, int height, float borderWidth, float borderRadius);

void buttonRenderer_setBackgroundTransparency(buttonRenderer* br, int transparentBackground);
void buttonRenderer_setFillColour(buttonRenderer* br, float r, float g, float b);
void buttonRenderer_setBorderColour(buttonRenderer* br, float r, float g, float b);
void buttonRenderer_setSize(buttonRenderer* br, int width, int height);

#endif