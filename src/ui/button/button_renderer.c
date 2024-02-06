#include "button_renderer.h"

#include <stdlib.h>
#include <glad/glad.h>

#include "../../shader/shader.h"
#include "../../glm2/mat4.h"

struct buttonRenderer {
	unsigned int vao;
	unsigned int vbo;
	unsigned int ebo;
	shader program;
};

static float vertices[8];
static unsigned int indices[6];

buttonRenderer* buttonRenderer_create(int width, int height)
{
	buttonRenderer* br = malloc(sizeof(buttonRenderer));

	glGenVertexArrays(1, &br->vao);
	glBindVertexArray(br->vao);

	glGenBuffers(1, &br->vbo);
	glGenBuffers(1, &br->ebo);

	glBindBuffer(GL_ARRAY_BUFFER, br->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, br->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	br->program = shader_import(
		"../assets/shaders/renderer2D/button/shader_button.vag", 
		"../assets/shaders/renderer2D/button/shader_button.fag", 
		NULL);
	glUseProgram(br->program.id);
	glUniformMatrix4fv(glGetUniformLocation(br->program.id, "projection"), 1, GL_FALSE, mat4_ortho(0, width, 0, height, -1, 1).data);
	glUniform3f(glGetUniformLocation(br->program.id, "fillColour"), 0.8f, 0.8f, 0.8f);
	glUniform3f(glGetUniformLocation(br->program.id, "borderColour"), 1, 1, 1);
	glUniform1i(glGetUniformLocation(br->program.id, "transparentBackground"), 0);
	glUseProgram(0);

	return br;
}

void buttonRenderer_destroy(buttonRenderer* br)
{
	shader_delete(&br->program);

	glDeleteVertexArrays(1, &br->vao);
	glDeleteBuffers(1, &br->vbo);
	glDeleteBuffers(1, &br->ebo);

	free(br);
}

void buttonRenderer_render(buttonRenderer* br, int x, int y, int width, int height, float borderWidth, float borderRadius)
{
	glUseProgram(br->program.id);
	glUniform4f(glGetUniformLocation(br->program.id, "pos_scale"), x, y, width, height);
	glUniform4f(glGetUniformLocation(br->program.id, "buttonData"), width, height, borderWidth, borderRadius);

	glBindVertexArray(br->vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

	glBindVertexArray(0);
	glUseProgram(0);
}

void buttonRenderer_setBackgroundTransparency(buttonRenderer* br, int transparentBackground)
{
	glUseProgram(br->program.id);
	glUniform1i(glGetUniformLocation(br->program.id, "transparentBackground"), transparentBackground);
	glUseProgram(0);
}

void buttonRenderer_setFillColour(buttonRenderer* br, float r, float g, float b)
{
	glUseProgram(br->program.id);
	glUniform3f(glGetUniformLocation(br->program.id, "fillColour"), r, g, b);
	glUseProgram(0);
}

void buttonRenderer_setBorderColour(buttonRenderer* br, float r, float g, float b)
{
	glUseProgram(br->program.id);
	glUniform3f(glGetUniformLocation(br->program.id, "borderColour"), r, g, b);
	glUseProgram(0);
}

void buttonRenderer_setSize(buttonRenderer* br, int width, int height)
{
	glUseProgram(br->program.id);
	glUniformMatrix4fv(glGetUniformLocation(br->program.id, "projection"), 1, GL_FALSE, mat4_ortho(0, width, 0, height, -1, 1).data);
	glUseProgram(0);
}

static float vertices[] = {
	0, 0,
	1, 0,
	1, 1,
	0, 1
};

static unsigned int indices[] = {
	0, 2, 1,
	3, 2, 0
};