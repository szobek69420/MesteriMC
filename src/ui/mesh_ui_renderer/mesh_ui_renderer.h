#ifndef MESH_UI_RENDERER_H
#define MESH_UI_RENDERER_H

struct meshUiRenderer;
typedef struct meshUiRenderer meshUiRenderer;

#define MESH_UI_RENDERER_VERTEX_FLOATS 6
#include "../../mesh/mesh.h"

/*
* the mesh ui renderer uses the following mesh format:
* location 0: vec3 position
* location 1: vec2 uv
* calls glDrawElements, therefore a GL_ELEMENT_ARRAY_BUFFER is needed
*/

meshUiRenderer* meshUiRenderer_create(int width, int height);
void meshUiRenderer_destroy(meshUiRenderer* mur);

void meshUiRenderer_render(meshUiRenderer* mur, mesh* m, unsigned int texture, float posX, float posY, float scaleX, float scaleY);

void meshUiRenderer_setSize(meshUiRenderer* mur, int width, int height);

#endif