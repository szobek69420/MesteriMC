#ifndef STARS_H
#define STARS_H

#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"

//designed to be rendered alongside the sky

struct stars;
typedef struct stars stars;

stars* stars_create(int starCount);
void stars_destroy(stars* s);

void stars_render(stars* s, mat4 pv);

void stars_setIntensity(stars* s, float intensity);
void stars_setPlayerPosition(stars* s, vec3 playerPos);

#endif