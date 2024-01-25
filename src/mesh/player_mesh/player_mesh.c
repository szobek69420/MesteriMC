#include "player_mesh.h"

#include "../mesh.h"
#include "../sphere/sphere.h"
#include "../../shader/shader.h"
#include "../../glm2/mat4.h"
#include "../../glm2/vec3.h"

#include <glad/glad.h>
#include <stdlib.h>
#include <math.h>

#define lerp(X,Y,I) ((X)+(I)*((Y)-(X)))

static float vertices_torso[264];
static float vertices_head[264];
static float vertices_arm[264];
static float vertices_leg[264];

static unsigned int indices[36];

static vec3 position_torso = { 0.0f, 1.05f, 0.0f };
static vec3 position_head = { 0.0f, 1.4f, 0.0f };
static vec3 position_arm_left = { -0.45f, 1.4f, 0.0f };
static vec3 position_arm_right = { 0.45f, 1.4f, 0.0f };
static vec3 position_leg_left = { -0.1f, 0.7f, 0.0f };
static vec3 position_leg_right = { 0.1f, 0.7f, 0.0f };

mesh create_mesh(float* vertices, unsigned int* indices, unsigned int vertexCount, unsigned int indexCount);

playerMesh playerMesh_create()
{
	playerMesh pm;
	pm.torso = create_mesh(vertices_torso, indices, 264,  36);
	pm.head = create_mesh(vertices_head, indices, 264, 36);
	pm.arm = create_mesh(vertices_arm, indices, 264, 36);
	pm.leg = create_mesh(vertices_leg, indices, 264, 36);

	pm.position = vec3_create(0);
	pm.rotX = 0; pm.rotY = 0;

	pm.rotTorsoX = 0; pm.rotTorsoY = 0;
	pm.rotHeadX = 0; pm.rotHeadY = 0;
	pm.rotArmLeftX = 0; pm.rotArmLeftY = 0;
	pm.rotArmRightX = 0; pm.rotArmRightY = 0;
	pm.rotLegLeftX = 0; pm.rotLegLeftY = 0;
	pm.rotLegRightX = 0; pm.rotLegRightX = 0;

	pm.animationTime = 0;

	pm.albedo = 0;
	pm.normal = 0;
	pm.specular = 0;

	playerMesh_calculateOuterModelMatrix(&pm);
	playerMesh_calculateInnerModelMatrices(&pm);

	return pm;
}

void playerMesh_destroy(playerMesh* pm)
{
	mesh_destroy(pm->torso);
	mesh_destroy(pm->head);
	mesh_destroy(pm->arm);
	mesh_destroy(pm->leg);
}

void playerMesh_calculateOuterModelMatrix(playerMesh* pm)
{
	pm->model = mat4_translate(mat4_create(1), pm->position);
	pm->model = mat4_rotate(pm->model, (vec3) { 0, 1, 0 }, pm->rotY);
}

void playerMesh_calculateInnerModelMatrices(playerMesh* pm)
{
	pm->modelTorso = mat4_translate(mat4_create(1), position_torso);
	pm->modelTorso= mat4_rotate(pm->modelTorso, (vec3) { 0, 1, 0 }, pm->rotTorsoY);
	pm->modelTorso = mat4_rotate(pm->modelTorso, (vec3) { 1, 0, 0 }, pm->rotTorsoX);

	pm->modelHead = mat4_translate(mat4_create(1), position_head);
	pm->modelHead = mat4_rotate(pm->modelHead, (vec3) { 0, 1, 0 }, pm->rotHeadY);
	pm->modelHead = mat4_rotate(pm->modelHead, (vec3) { 1, 0, 0 }, pm->rotHeadX);

	pm->modelArmLeft = mat4_translate(mat4_create(1), position_arm_left);
	pm->modelArmLeft = mat4_rotate(pm->modelArmLeft, (vec3) { 0, 1, 0 }, pm->rotArmLeftY);
	pm->modelArmLeft = mat4_rotate(pm->modelArmLeft, (vec3) { 1, 0, 0 }, pm->rotArmLeftX);

	pm->modelArmRight = mat4_translate(mat4_create(1), position_arm_right);
	pm->modelArmRight = mat4_rotate(pm->modelArmRight, (vec3) { 0, 1, 0 }, pm->rotArmRightY);
	pm->modelArmRight = mat4_rotate(pm->modelArmRight, (vec3) { 1, 0, 0 }, pm->rotArmRightX);

	pm->modelLegLeft = mat4_translate(mat4_create(1), position_leg_left);
	pm->modelLegLeft = mat4_rotate(pm->modelLegLeft, (vec3) { 0, 1, 0 }, pm->rotLegLeftY);
	pm->modelLegLeft = mat4_rotate(pm->modelLegLeft, (vec3) { 1, 0, 0 }, pm->rotLegLeftX);

	pm->modelLegRight = mat4_translate(mat4_create(1), position_leg_right);
	pm->modelLegRight = mat4_rotate(pm->modelLegRight, (vec3) { 0, 1, 0 }, pm->rotLegRightY);
	pm->modelLegRight = mat4_rotate(pm->modelLegRight, (vec3) { 1, 0, 0 }, pm->rotLegRightX);
}

void playerMesh_animate(playerMesh* pm, unsigned int animation, float deltaTime)
{
	switch (animation)
	{
	case PLAYER_MESH_ANIMATION_WALK:
		pm->animationTime += deltaTime;
		
		pm->rotTorsoX = lerp(pm->rotTorsoX, 0, 0.2f);
		pm->rotTorsoY = lerp(pm->rotTorsoY, 0, 0.2f);

		//a fejet nem allitja be, mert arra nem hat az animacio

		pm->rotArmLeftX = lerp(pm->rotArmLeftX, 45 * sinf(PLAYER_MESH_ANIMATION_SPEED*pm->animationTime), 0.2f);
		pm->rotArmLeftY = lerp(pm->rotArmLeftY, 0, 0.2f);
		pm->rotArmRightX = lerp(pm->rotArmRightX, 45 * sinf(-PLAYER_MESH_ANIMATION_SPEED * pm->animationTime), 0.2f);
		pm->rotArmRightY = lerp(pm->rotArmRightY, 0, 0.2f);

		pm->rotLegLeftX = lerp(pm->rotLegLeftX, 45 * sinf(-PLAYER_MESH_ANIMATION_SPEED * pm->animationTime), 0.2f);
		pm->rotLegLeftY = lerp(pm->rotLegLeftY, 0, 0.2f);
		pm->rotLegRightX = lerp(pm->rotLegRightX, 45 * sinf(PLAYER_MESH_ANIMATION_SPEED * pm->animationTime), 0.2f);
		pm->rotLegRightY = lerp(pm->rotLegRightY, 0, 0.2f);
		break;

	default://idle
		pm->animationTime =0;

		pm->rotTorsoX = lerp(pm->rotTorsoX, 0, 0.2f);
		pm->rotTorsoY = lerp(pm->rotTorsoY, 0, 0.2f);

		//a fejet nem allitja be, mert arra nem hat az animacio

		pm->rotArmLeftX = lerp(pm->rotArmLeftX, 0, 0.2f);
		pm->rotArmLeftY = lerp(pm->rotArmLeftY, 0, 0.2f);
		pm->rotArmRightX = lerp(pm->rotArmRightX, 0, 0.2f);
		pm->rotArmRightY = lerp(pm->rotArmRightY, 0, 0.2f);

		pm->rotLegLeftX = lerp(pm->rotLegLeftX, 0, 0.2f);
		pm->rotLegLeftY = lerp(pm->rotLegLeftY, 0, 0.2f);
		pm->rotLegRightX = lerp(pm->rotLegRightX, 0, 0.2f);
		pm->rotLegRightY = lerp(pm->rotLegRightY, 0, 0.2f);
		break;
	}
}

void playerMesh_render(playerMesh* pm, shader* shit)
{
	glUseProgram(shit->id);

	unsigned int modelLocation = glGetUniformLocation(shit->id, "model");

	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pm->albedo);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pm->normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, pm->specular);
	

	//torso
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, mat4_multiply(pm->model, pm->modelTorso).data);
	glBindVertexArray(pm->torso.vao);
	glDrawElements(GL_TRIANGLES, pm->torso.indexCount, GL_UNSIGNED_INT, NULL);

	//head
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, mat4_multiply(pm->model, pm->modelHead).data);
	glBindVertexArray(pm->head.vao);
	glDrawElements(GL_TRIANGLES, pm->head.indexCount, GL_UNSIGNED_INT, NULL);

	//arms
	glBindVertexArray(pm->arm.vao);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, mat4_multiply(pm->model, pm->modelArmLeft).data);
	glDrawElements(GL_TRIANGLES, pm->arm.indexCount, GL_UNSIGNED_INT, NULL);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, mat4_multiply(pm->model, pm->modelArmRight).data);
	glDrawElements(GL_TRIANGLES, pm->arm.indexCount, GL_UNSIGNED_INT, NULL);

	//legs
	glBindVertexArray(pm->leg.vao);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, mat4_multiply(pm->model, pm->modelLegLeft).data);
	glDrawElements(GL_TRIANGLES, pm->leg.indexCount, GL_UNSIGNED_INT, NULL);
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, mat4_multiply(pm->model, pm->modelLegRight).data);
	glDrawElements(GL_TRIANGLES, pm->leg.indexCount, GL_UNSIGNED_INT, NULL);

	glUseProgram(0);
	glBindVertexArray(0);
}

mesh create_mesh(float* vertices, unsigned int* indices, unsigned int vertexCount, unsigned int indexCount)
{
	mesh meh;

	glGenVertexArrays(1, &meh.vao);
	glBindVertexArray(meh.vao);

	glGenBuffers(1, &meh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, meh.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(float), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &meh.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meh.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount*sizeof(unsigned int), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);//positions
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);//uv and geom
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);//normal
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);//tangent

	glBindVertexArray(0);

	meh.indexCount = indexCount;

	return meh;
}

static float vertices_reference[] = {
	//position(3), uv(2), normal(3), tangent(3)
	0,0,1,	0,0,	0,0,1,	1,0,0,
	1,0,1,	0,0,	0,0,1,	1,0,0,
	1,1,1,	0,0,	0,0,1,	1,0,0,
	0,1,1,	0,0,	0,0,1,	1,0,0,

	1,0,1,	0,0,	1,0,0,	0,0,-1,
	1,0,0,	0,0,	1,0,0,	0,0,-1,
	1,1,0,	0,0,	1,0,0,	0,0,-1,
	1,1,1,	0,0,	1,0,0,	0,0,-1,

	1,0,0,	0,0,	0,0,-1,	-1,0,0,
	0,0,0,	0,0,	0,0,-1,	-1,0,0,
	0,1,0,	0,0,	0,0,-1,	-1,0,0,
	1,1,0,	0,0,	0,0,-1,	-1,0,0,

	0,0,0,	0,0,	-1,0,0,	0,0,1,
	0,0,1,	0,0,	-1,0,0,	0,0,1,
	0,1,1,	0,0,	-1,0,0,	0,0,1,
	0,1,0,	0,0,	-1,0,0,	0,0,1,

	0,1,1,	0,0,	0,1,0,	1,0,0,
	1,1,1,	0,0,	0,1,0,	1,0,0,
	1,1,0,	0,0,	0,1,0,	1,0,0,
	0,1,0,	0,0,	0,1,0,	1,0,0,

	0,0,0,	0,0,	0,-1,0,	1,0,0,
	1,0,0,	0,0,	0,-1,0,	1,0,0,
	1,0,1,	0,0,	0,-1,0,	1,0,0,
	0,0,1,	0,0,	0,-1,0,	1,0,0
};

static float vertices_torso[] = {
	//position(3), uv(2), normal(3), tangent(3)
	-0.2f,-0.35f,0.1f,	0,0,	0,0,1,	1,0,0,
	0.2f,-0.35f,0.1f,	0,0,	0,0,1,	1,0,0,
	0.4f,0.35f,0.1f,	0,0,	0,0,1,	1,0,0,
	-0.4f,0.35f,0.1f,	0,0,	0,0,1,	1,0,0,

	0.2f,-0.35f,0.1f,	0,0,	1,0,0,	0,0,-1,
	0.2f,-0.35f,-0.1f,	0,0,	1,0,0,	0,0,-1,
	0.4f,0.35f,-0.1f,	0,0,	1,0,0,	0,0,-1,
	0.4f,0.35f,0.1f,	0,0,	1,0,0,	0,0,-1,

	0.2f,-0.35f,-0.1f,	0,0,	0,0,-1,	-1,0,0,
	-0.2f,-0.35f,-0.1f,	0,0,	0,0,-1,	-1,0,0,
	-0.4f,0.35f,-0.1f,	0,0,	0,0,-1,	-1,0,0,
	0.4f,0.35f,-0.1f,	0,0,	0,0,-1,	-1,0,0,

	-0.2f,-0.35f,-0.1f,	0,0,	-1,0,0,	0,0,1,
	-0.2f,-0.35f,0.1f,	0,0,	-1,0,0,	0,0,1,
	-0.4f,0.35f,0.1f,	0,0,	-1,0,0,	0,0,1,
	-0.4f,0.35f,-0.1f,	0,0,	-1,0,0,	0,0,1,

	-0.4f,0.35f,0.1f,	0,0,	0,1,0,	1,0,0,
	0.4f,0.35f,0.1f,	0,0,	0,1,0,	1,0,0,
	0.4f,0.35f,-0.1f,	0,0,	0,1,0,	1,0,0,
	-0.4f,0.35f,-0.1f,	0,0,	0,1,0,	1,0,0,

	-0.2f,-0.35f,-0.1f,	0,0,	0,-1,0,	1,0,0,
	0.2f,-0.35f,-0.1f,	0,0,	0,-1,0,	1,0,0,
	0.2f,-0.35f,0.1f,	0,0,	0,-1,0,	1,0,0,
	-0.2f,-0.35f,0.1f,	0,0,	0,-1,0,	1,0,0
};

static float vertices_head[] = {
	//position(3), uv(2), normal(3), tangent(3)
	-0.2f,0,0.2f,	0,0,	0,0,1,	1,0,0,
	0.2f,0,0.2f,	0,0,	0,0,1,	1,0,0,
	0.2f,0.4f,0.2f,	0,0,	0,0,1,	1,0,0,
	-0.2f,0.4f,0.2f,	0,0,	0,0,1,	1,0,0,

	0.2f,0,0.2f,	0,0,	1,0,0,	0,0,-1,
	0.2f,0,-0.2f,	0,0,	1,0,0,	0,0,-1,
	0.2f,0.4f,-0.2f,	0,0,	1,0,0,	0,0,-1,
	0.2f,0.4f,0.2f,	0,0,	1,0,0,	0,0,-1,

	0.2f,0,-0.2f,	0,0,	0,0,-1,	-1,0,0,
	-0.2f,0,-0.2f,	0,0,	0,0,-1,	-1,0,0,
	-0.2f,0.4f,-0.2f,	0,0,	0,0,-1,	-1,0,0,
	0.2f,0.4f,-0.2f,	0,0,	0,0,-1,	-1,0,0,

	-0.2f,0,-0.2f,	0,0,	-1,0,0,	0,0,1,
	-0.2f,0,0.2f,	0,0,	-1,0,0,	0,0,1,
	-0.2f,0.4f,0.2f,	0,0,	-1,0,0,	0,0,1,
	-0.2f,0.4f,-0.2f,	0,0,	-1,0,0,	0,0,1,

	-0.2f,0.4f,0.2f,	0,0,	0,1,0,	1,0,0,
	0.2f,0.4f,0.2f,	0,0,	0,1,0,	1,0,0,
	0.2f,0.4f,-0.2f,	0,0,	0,1,0,	1,0,0,
	-0.2f,0.4f,-0.2f,	0,0,	0,1,0,	1,0,0,

	-0.2f,0,-0.2f,	0,0,	0,-1,0,	1,0,0,
	0.2f,0,-0.2f,	0,0,	0,-1,0,	1,0,0,
	0.2f,0,0.2f,	0,0,	0,-1,0,	1,0,0,
	-0.2f,0,0.2f,	0,0,	0,-1,0,	1,0,0
};

static float vertices_arm[] = {
	//position(3), uv(2), normal(3), tangent(3)
	-0.1f,-0.7f,0.1f,	0,0,	0,0,1,	1,0,0,
	0.1f,-0.7f,0.1f,	0,0,	0,0,1,	1,0,0,
	0.1f,0.0f,0.1f,	0,0,	0,0,1,	1,0,0,
	-0.1f,0.0f,0.1f,	0,0,	0,0,1,	1,0,0,

	0.1f,-0.7f,0.1f,	0,0,	1,0,0,	0,0,-1,
	0.1f,-0.7f,-0.1f,	0,0,	1,0,0,	0,0,-1,
	0.1f,0.0f,-0.1f,	0,0,	1,0,0,	0,0,-1,
	0.1f,0.0f,0.1f,	0,0,	1,0,0,	0,0,-1,

	0.1f,-0.7f,-0.1f,	0,0,	0,0,-1,	-1,0,0,
	-0.1f,-0.7f,-0.1f,	0,0,	0,0,-1,	-1,0,0,
	-0.1f,0.0f,-0.1f,	0,0,	0,0,-1,	-1,0,0,
	0.1f,0.0f,-0.1f,	0,0,	0,0,-1,	-1,0,0,

	-0.1f,-0.7f,-0.1f,	0,0,	-1,0,0,	0,0,1,
	-0.1f,-0.7f,0.1f,	0,0,	-1,0,0,	0,0,1,
	-0.1f,0.0f,0.1f,	0,0,	-1,0,0,	0,0,1,
	-0.1f,0.0f,-0.1f,	0,0,	-1,0,0,	0,0,1,

	-0.1f,0.0f,0.1f,	0,0,	0,1,0,	1,0,0,
	0.1f,0.0f,0.1f,		0,0,	0,1,0,	1,0,0,
	0.1f,0.0f,-0.1f,	0,0,	0,1,0,	1,0,0,
	-0.1f,0.0f,-0.1f,	0,0,	0,1,0,	1,0,0,

	-0.1f,-0.7f,-0.1f,	0,0,	0,-1,0,	1,0,0,
	0.1f,-0.7f,-0.1f,	0,0,	0,-1,0,	1,0,0,
	0.1f,-0.7f,0.1f,	0,0,	0,-1,0,	1,0,0,
	-0.1f,-0.7f,0.1f,	0,0,	0,-1,0,	1,0,0
};

static float vertices_leg[] = {
	//position(3), uv(2), normal(3), tangent(3)
	-0.1f,-0.7f,0.1f,	0,0,	0,0,1,	1,0,0,
	0.1f,-0.7f,0.1f,	0,0,	0,0,1,	1,0,0,
	0.1f,0.0f,0.1f,	0,0,	0,0,1,	1,0,0,
	-0.1f,0.0f,0.1f,	0,0,	0,0,1,	1,0,0,

	0.1f,-0.7f,0.1f,	0,0,	1,0,0,	0,0,-1,
	0.1f,-0.7f,-0.1f,	0,0,	1,0,0,	0,0,-1,
	0.1f,0.0f,-0.1f,	0,0,	1,0,0,	0,0,-1,
	0.1f,0.0f,0.1f,	0,0,	1,0,0,	0,0,-1,

	0.1f,-0.7f,-0.1f,	0,0,	0,0,-1,	-1,0,0,
	-0.1f,-0.7f,-0.1f,	0,0,	0,0,-1,	-1,0,0,
	-0.1f,0.0f,-0.1f,	0,0,	0,0,-1,	-1,0,0,
	0.1f,0.0f,-0.1f,	0,0,	0,0,-1,	-1,0,0,

	-0.1f,-0.7f,-0.1f,	0,0,	-1,0,0,	0,0,1,
	-0.1f,-0.7f,0.1f,	0,0,	-1,0,0,	0,0,1,
	-0.1f,0.0f,0.1f,	0,0,	-1,0,0,	0,0,1,
	-0.1f,0.0f,-0.1f,	0,0,	-1,0,0,	0,0,1,

	-0.1f,0.0f,0.1f,	0,0,	0,1,0,	1,0,0,
	0.1f,0.0f,0.1f,		0,0,	0,1,0,	1,0,0,
	0.1f,0.0f,-0.1f,	0,0,	0,1,0,	1,0,0,
	-0.1f,0.0f,-0.1f,	0,0,	0,1,0,	1,0,0,

	-0.1f,-0.7f,-0.1f,	0,0,	0,-1,0,	1,0,0,
	0.1f,-0.7f,-0.1f,	0,0,	0,-1,0,	1,0,0,
	0.1f,-0.7f,0.1f,	0,0,	0,-1,0,	1,0,0,
	-0.1f,-0.7f,0.1f,	0,0,	0,-1,0,	1,0,0
};

static unsigned int indices[] = {
	0,2,1,	3,2,0,
	4,6,5,	7,6,4,
	8,10,9,	11,10,8,
	12,14,13,	15,14,12,
	16,18,17,	19,18,16,
	20,22,21,	23,22,20
};