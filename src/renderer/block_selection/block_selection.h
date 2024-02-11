#ifndef BLOCK_SELECTION_H
#define BLOCK_SELECTION_H


#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"

void blockSelection_init();
void blockSelection_close();

void blockSelection_render(vec3 position, vec3 size, mat4* pv);

void blockSelection_setColour(float r, float g, float b);

#endif