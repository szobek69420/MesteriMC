#include "block_selection.h"

#include "../../shader/shader.h"

#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"

#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>

static float vertices[24];
static unsigned int indices[36];

static struct {
	int initialized;
	unsigned int vao;
	unsigned int vbo;
	unsigned int ebo;
	shader program;
} bs = { 0,0,0,0 };

void blockSelection_init()
{
	bs.program = shader_import(
		"../assets/shaders/random/block_selection/shader_block_selection.vag",
		"../assets/shaders/random/block_selection/shader_block_selection.fag",
		NULL
	);
	blockSelection_setColour(0, 0, 0);
	

	glGenVertexArrays(1, &bs.vao);
	glBindVertexArray(bs.vao);

	glGenBuffers(1, &bs.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, bs.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &bs.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bs.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	bs.initialized = 69;
}

void blockSelection_close()
{
	shader_delete(&bs.program);

	glDeleteVertexArrays(1, &bs.vao);
	glDeleteBuffers(1, &bs.vbo);
	glDeleteBuffers(1, &bs.ebo);

	bs.initialized = 0;
}

void blockSelection_render(vec3 position, vec3 size, mat4* pv)
{
	if (bs.initialized == 0)
	{
		fprintf(stderr, "tried to render block selection without initialization L\n");
		return;
	}

	position = vec3_sum(position, vec3_scale(size, -0.5f));

	glDepthFunc(GL_LEQUAL);
	glLineWidth(3);

	glUseProgram(bs.program.id);
	glUniform3f(glGetUniformLocation(bs.program.id, "pos"), position.x, position.y, position.z);
	glUniform3f(glGetUniformLocation(bs.program.id, "size"), size.x, size.y, size.z);
	glUniformMatrix4fv(glGetUniformLocation(bs.program.id, "pv"), 1, GL_FALSE, pv->data);

	glBindVertexArray(bs.vao);
	glDrawElements(GL_LINES,36,GL_UNSIGNED_INT,0);
	glBindVertexArray(0);

	glUseProgram(0);
	glDepthFunc(GL_LESS);
}

void blockSelection_setColour(float r, float g, float b)
{
	if (bs.initialized == 0)
		return;

	glUseProgram(bs.program.id);
	glUniform3f(glGetUniformLocation(bs.program.id, "colour"), r, g, b);
	glUseProgram(0);
}

static float vertices[] = {
	0, 1, 1,	//0
	1, 1, 1,	//1
	1, 1, 0,	//2
	0, 1, 0,	//3
	0, 0, 1,	//4
	1, 0, 1,	//5
	1, 0, 0,	//6
	0, 0, 0		//7
};

static unsigned int indices[] = {
	0,1,	3,2,	4,5,	7,6,
	4,0,	5,1,	6,2,	7,3,
	2,1,	3,0,	6,5,	7,4
};