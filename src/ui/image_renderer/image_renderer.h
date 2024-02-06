#ifndef IMAGE_RENDERER_H
#define IMAGE_RENDERER_H

struct imageRenderer;
typedef struct imageRenderer imageRenderer;

imageRenderer* imageRenderer_create(int width, int height);
void imageRenderer_destroy(imageRenderer* ir);

void imageRenderer_render(imageRenderer* ir, unsigned int textureId, int x, int y, int width, int height, float uvx, float uvy, float uvWidth, float uvHeight);

void imageRenderer_setTint(imageRenderer* ir, float r, float g, float b);
void imageRenderer_setSize(imageRenderer* ir, int width, int height);

#endif