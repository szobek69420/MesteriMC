#ifndef SUN_H
#define SUN_H

#include "../../camera/camera.h"
#include "../../mesh/mesh.h"
#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"
#include "../../shader/shader.h"

#define SUN_SIZE 30

typedef struct {
	mesh meh;
	shader program;

	vec3 direction;
	mat4 model;
} sun;

sun sun_create();
void sun_destroy(sun* sunTzu);

void sun_render(sun* sunTzu, camera* cum, mat4* projection);

void sun_setDirection(sun* sunTzu, vec3 direction);

#endif