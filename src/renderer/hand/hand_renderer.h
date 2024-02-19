#ifndef HAND_RENDERER_H
#define HAND_RENDERER_H

#include "../../glm2/mat4.h"

#define HAND_RENDERER_BLOCK 69
#define HAND_RENDERER_ITEM 420

struct handRenderer;
typedef struct handRenderer handRenderer;


handRenderer* handRenderer_create();
void handRenderer_destroy(handRenderer* hr);

void handRenderer_renderBlock(handRenderer* hr, mat4* model);
void handRenderer_renderItem(handRenderer* hr, mat4* model, mat4* model_normal);

void handRenderer_setMatrices(handRenderer* hr, mat4* proj, mat4* view, mat3* view_normal);

void handRenderer_setBlock(handRenderer* hr, int blockType);
void handRenderer_setItem(handRenderer* hr, int itemType);

#endif