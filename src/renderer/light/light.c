#include "light.h"
#include "../../glm2/mat4.h"
#include "../../glm2/vec3.h"

#include "../../mesh/sphere/sphere.h"

#include <math.h>
#include <stdio.h>

#include <glad/glad.h>

light light_create(vec3 colour, vec3 position, vec3 attenuation)
{
	light romogus;
	romogus.colour = colour;

	romogus.attenuation = attenuation;
	if (attenuation.y < 0.0001 && attenuation.z < 0.0001)
		romogus.radius = -69;
	else
		romogus.radius = (-attenuation.y + sqrtf(attenuation.y * attenuation.y - 4 * attenuation.z * (1 - (256.0f / 20.0f) * attenuation.x))) / (2.0f * attenuation.z);

	romogus.position = position;
	romogus.model = mat4_create2((float[]){
		romogus.radius, 0, 0, 0,
		0, romogus.radius, 0, 0,
		0, 0, romogus.radius, 0,
		position.x, position.y, position.z, 1
		});

	return romogus;
}

void light_setPosition(light* lit, vec3 position)
{
	lit->position = position;
	lit->model = mat4_create2((float[]) {
			lit->radius, 0, 0, 0,
			0, lit->radius, 0, 0,
			0, 0, lit->radius, 0,
			position.x, position.y, position.z, 1
	});
}

void light_setAttenuation(light* lit, vec3 attenuation)
{
	lit->attenuation = attenuation;
	if (attenuation.y < 0.001 && attenuation.z < 0.001)
		lit->radius = -69;
	else
		lit->radius = (-attenuation.y + sqrtf(attenuation.y * attenuation.y - 4 * attenuation.z * (1 - (256.0f /20.0f) * attenuation.x))) / (2.0f * attenuation.z);

	lit->model = mat4_create2((float[]) {
			lit->radius, 0, 0, 0,
			0, lit->radius, 0, 0,
			0, 0, lit->radius, 0,
			lit->position.x, lit->position.y, lit->position.z, 1
	});
}

unsigned int light_createInstanceVBO()
{
	unsigned int vbo;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, LIGHT_MAX_NUMBER * LIGHT_SIZE_IN_VBO, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return vbo;
}

light_renderer light_createRenderer()
{
	light_renderer lr;

	lr.currentLightCount = 0;
	lr.instanceVBO = light_createInstanceVBO();

	lr.pointMesh = sphere_create();
	glBindVertexArray(lr.pointMesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, lr.instanceVBO);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, LIGHT_SIZE_IN_VBO, (void*)0);//light position in view space
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, LIGHT_SIZE_IN_VBO, (void*)(3*sizeof(float)));//light colour
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, LIGHT_SIZE_IN_VBO, (void*)(6 * sizeof(float)));//light attenuation
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, LIGHT_SIZE_IN_VBO, (void*)(9 * sizeof(float)));//model matrix 1. column
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, LIGHT_SIZE_IN_VBO, (void*)(13 * sizeof(float)));//model matrix 2. column
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, LIGHT_SIZE_IN_VBO, (void*)(17 * sizeof(float)));//model matrix 3. column
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, LIGHT_SIZE_IN_VBO, (void*)(21 * sizeof(float)));//model matrix 4. column
	glEnableVertexAttribArray(7);

	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//TODO: directional mesh

	return lr;
}

void light_destroyRenderer(light_renderer lr)
{
	glDeleteBuffers(1, &lr.instanceVBO);

	mesh_destroy(lr.pointMesh);
	//TODO: directional mesh
}

void light_fillRenderer(light_renderer* lr, float* data, unsigned int lightCount)
{
	glBindBuffer(GL_ARRAY_BUFFER, lr->instanceVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, LIGHT_SIZE_IN_VBO * lightCount, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	lr->currentLightCount = lightCount;
}

void light_render(light_renderer* lr, int pointLights)
{
	if (pointLights!=0)
	{
		glBindVertexArray(lr->pointMesh.vao);
		glDrawElementsInstanced(GL_TRIANGLES, lr->pointMesh.indexCount, GL_UNSIGNED_INT, 0, lr->currentLightCount);
		glBindVertexArray(0);
	}
}