#include "stars.h"

#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "glad/glad.h"

#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"
#include "../../shader/shader.h"
#include "../../texture_handler/texture_handler.h"

struct stars {
	unsigned int vao;
	unsigned int vbo, ebo, instanceVbo;

	shader program;

	int starCount;
};

static float vertices[] = {
	-0.1f, -0.1f, 100.0f,	0,0,
	0.1f, -0.1f, 100.0f,	1,0,
	0.1f, 0.1f, 100.0f,		1,1,
	-0.1f, 0.1f, 100.0f,	0,1
};

static unsigned int indices[] = {
	0,1,2, 2,3,0
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
		float rotX = 360.0f * powf(((float)rand() / (float)RAND_MAX),2);
		float rotY = 360.0f * ((float)rand() / (float)RAND_MAX);
		float scale = 1.0f + 3.0f * ((float)rand() / (float)RAND_MAX);

		instanceMatrices[i] = mat4_create(1);
		instanceMatrices[i] = mat4_rotate(instanceMatrices[i], (vec3) { 0, 1, 0 }, rotY);
		instanceMatrices[i] = mat4_rotate(instanceMatrices[i], (vec3) { 1, 0, 0 }, rotX);
		instanceMatrices[i] = mat4_multiply(instanceMatrices[i], (mat4) { scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 });
	}

	//do opengl things
	s->program = shader_import(
		"../assets/shaders/renderer/stars/shader_stars.vag", 
		"../assets/shaders/renderer/stars/shader_stars.fag", 
		NULL);
	glUseProgram(s->program.id);
	glUniform1i(glGetUniformLocation(s->program.id, "texture_star"), 0);
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3*sizeof(float));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);


	glBindBuffer(GL_ARRAY_BUFFER, s->instanceVbo);
	glBufferData(GL_ARRAY_BUFFER, starCount*16 * sizeof(float), instanceMatrices, GL_STATIC_DRAW);

	glBindVertexArray(s->vao);

	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), 0);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), 4 * sizeof(float));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), 8 * sizeof(float));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), 12 * sizeof(float));
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);

	glBindVertexArray(0);

	s->starCount = starCount;

	free(instanceMatrices);

	return s;
}

void stars_destroy(stars* s)
{
	glDeleteVertexArrays(1, &s->vao);
	glDeleteBuffers(1, &s->vbo);
	glDeleteBuffers(1, &s->ebo);
	glDeleteBuffers(1, &s->instanceVbo);

	shader_delete(&s->program);

	free(s);
}

void stars_render(stars* s, mat4 pv)
{
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(s->program.id);
	glUniformMatrix4fv(glGetUniformLocation(s->program.id, "pv"), 1, GL_FALSE, pv.data);
	glUniformMatrix4fv(glGetUniformLocation(s->program.id, "model"), 1, GL_FALSE, mat4_create(1).data);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_STAR));

	glBindVertexArray(s->vao);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, s->starCount);

	glBindVertexArray(0);
	glUseProgram(0);

	glEnable(GL_DEPTH_TEST);
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
