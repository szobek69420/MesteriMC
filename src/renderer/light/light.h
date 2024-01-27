#ifndef LIGHT_H
#define LIGHT_H

#include "../../mesh/mesh.h"

#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"

#define LIGHT_MAX_NUMBER 100
#define LIGHT_SIZE_IN_VBO 25*sizeof(float)//position, colour, attenuation, model matrix
#define LIGHT_FLOATS_IN_VBO 25

//ha directional light, akkor a position az a direction és a range az negatív (directional light, ha a linear és a quadratic 0.00001 vagy kevesebb)
typedef struct light {
	vec3 position;
	vec3 colour;
	vec3 attenuation;//(intensity, linear, quadratic)  NOTE: constant=1
	mat4 model; //a deferred lighting eseten a gombot transzformalja
	float radius;
} light;//fontos a sorrend, mert a shader ebben a sorrendben várja õket

typedef struct light_renderer {
	mesh pointMesh;
	mesh directionalMesh;

	unsigned int instanceVBO;
	unsigned int currentLightCount;
} light_renderer;


light light_create(vec3 colour, vec3 position, vec3 attenuation);

void light_setPosition(light* lit, vec3 position);
void light_setAttenuation(light* lit, vec3 attenuation);


light_renderer light_createRenderer();
void light_destroyRenderer(light_renderer lr);
void light_fillRenderer(light_renderer* lr, float* data, unsigned int lightCount);

void light_render(light_renderer* lr, int pointLights);

#endif