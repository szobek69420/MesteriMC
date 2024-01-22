#ifndef PLAYER_MESH_H
#define PLAYER_MESH_H

#include "../mesh.h"
#include "../../shader/shader.h"
#include "../../glm2/mat4.h"
#include "../../glm2/vec3.h"

typedef struct {
	mesh torso;
	mesh head;
	mesh arm;
	mesh leg;

	mat4 model;
	mat4 modelTorso;
	mat4 modelHead;
	mat4 modelArmLeft;
	mat4 modelArmRight;
	mat4 modelLegLeft;
	mat4 modelLegRight;

	vec3 position;
	float rotX, rotY;
	
	float rotTorsoX, rotTorsoY;
	float rotHeadX, rotHeadY;
	float rotArmLeftX, rotArmLeftY;
	float rotArmRightX, rotArmRightY;
	float rotLegLeftX, rotLegLeftY;
	float rotLegRightX, rotLegRightY;

	unsigned int albedo;
	unsigned int specular;
	unsigned int normal;
} playerMesh;

playerMesh playerMesh_create();
void playerMesh_destroy(playerMesh* pm);

void playerMesh_calculateOuterModelMatrix(playerMesh* pm);
void playerMesh_calculateInnerModelMatrices(playerMesh* pm);

void playerMesh_render(playerMesh* pm, shader* shit);

#endif