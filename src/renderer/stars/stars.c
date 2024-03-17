#include "stars.h"

#include <stdlib.h>
#include <time.h>

#include "glad/glad.h"

#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"
#include "../../shader/shader.h"

struct stars {
	unsigned int vao;
	unsigned int vbo, ebo, instanceVbo;

	shader program;

	int starCount;
};

static float vertices[] = {
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f
};

static unsigned int indices[] = {
	1,0,2, 2,3,0
};

stars* stars_create(int starCount)
{
	//alloc stars
	stars* s = malloc(sizeof(stars));

	//generate matrices
	srand(time(0));
	mat4* instanceMatrices = malloc(starCount * sizeof(mat4));
	for (unsigned int i = 0; i < starCount; i++)
	{
		float rotX = 90.0f * ((float)rand() / (float)RAND_MAX);
		float rotY = 360.0f * ((float)rand() / (float)RAND_MAX);

		instanceMatrices[i] = mat4_create(1);
		instanceMatrices[i] = mat4_rotate(instanceMatrices[i], (vec3) { 0, 1, 0 }, rotY);
		instanceMatrices[i] = mat4_rotate(instanceMatrices[i], (vec3) { 1, 0, 0 }, rotX);
	}

	//do opengl things
	s->program = shader_import(
		"../assets/shaders/renderer/stars/shader_stars.vag", 
		"../assets/shaders/renderer/stars/shader_stars.fag", 
		NULL);

	stars_setIntensity(s, 0);
	stars_setPlayerPosition(s, (vec3) { 0, 0, 0 });

	glGenVertexArrays(1, &s->vao);
	glGenBuffers(1, &s->vbo);
	glGenBuffers(1, &s->ebo);
	glGenBuffers(1, &s->instanceVbo);

	glBindVertexArray(s->vao);
	glBindBuffer(GL_ARRAY_BUFFER, s->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, s->instanceVbo);
	glBufferData(GL_ARRAY_BUFFER, starCount*16 * sizeof(float), instanceMatrices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), 0);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), 4 * sizeof(float));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), 8 * sizeof(float));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), 12 * sizeof(float));
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	s->starCount = starCount;

	free(instanceMatrices);

	return s;
}

void stars_destroy(stars* s)
{
	glDeleteVertexArrays(1, &s->vao);
	glDeleteBuffers(1, &s->vbo);
	glDeleteBuffers(1, &s->instanceVbo);

	shader_delete(&s->program);

	free(s);
}

void stars_render(stars* s, mat4 pv)
{
	glEnable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);

	glUseProgram(s->program.id);
	glUniformMatrix4fv(glGetUniformLocation(s->program.id, "pv"), 1, GL_FALSE, pv.data);

	glBindVertexArray(s->vao);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, s->starCount);

	glBindVertexArray(0);
	glUseProgram(0);

	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
}

void stars_setIntensity(stars* s, float intensity)
{
	glUseProgram(s->program.id);
	glUniform1f(glGetUniformLocation(s->program.id, "intensity"), intensity);
	glUseProgram(0);
}

void stars_setPlayerPosition(stars* s, vec3 playerPos)
{
	glUseProgram(s->program.id);
	glUniform3f(glGetUniformLocation(s->program.id, "playerPos"), playerPos.x, playerPos.y, playerPos.z);
	glUseProgram(0);
}
