#include "image_renderer.h"

#include "../../shader/shader.h"
#include "../../glm2/mat4.h"
#include <glad/glad.h>
#include <stdlib.h>

struct imageRenderer {
	unsigned int vao;
	unsigned int vbo;
	unsigned int ebo;
	shader program;
};

static float vertices[8];
static unsigned int indices[6];

imageRenderer* imageRenderer_create(int width, int height)
{
	imageRenderer* ir = malloc(sizeof(imageRenderer));

	glGenVertexArrays(1, &ir->vao);
	glBindVertexArray(ir->vao);

	glGenBuffers(1, &ir->vbo);
	glGenBuffers(1, &ir->ebo);

	glBindBuffer(GL_ARRAY_BUFFER, ir->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ir->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	ir->program = shader_import(
		"../assets/shaders/renderer2D/image/shader_image.vag",
		"../assets/shaders/renderer2D/image/shader_image.fag",
		NULL);
	glUseProgram(ir->program.id);
	glUniformMatrix4fv(glGetUniformLocation(ir->program.id, "projection"), 1, GL_FALSE, mat4_ortho(0, width, 0, height, -1, 1).data);
	glUniform3f(glGetUniformLocation(ir->program.id, "tint"), 1, 1, 1);
	glUniform1i(glGetUniformLocation(ir->program.id, "tex"), 0);
	glUseProgram(0);

	return ir;
}
void imageRenderer_destroy(imageRenderer* ir)
{
	shader_delete(&ir->program);

	glDeleteVertexArrays(1, &ir->vao);
	glDeleteBuffers(1, &ir->vbo);
	glDeleteBuffers(1, &ir->ebo);

	free(ir);
}

void imageRenderer_render(imageRenderer* ir, unsigned int textureId, int x, int y, int width, int height, float uvx, float uvy, float uvWidth, float uvHeight)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glUseProgram(ir->program.id);
	glUniform4f(glGetUniformLocation(ir->program.id, "pos_scale"), x, y, width, height);
	glUniform4f(glGetUniformLocation(ir->program.id, "uv_scale"), uvx, uvy, uvWidth, uvHeight);

	glBindVertexArray(ir->vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

	glBindVertexArray(0);
	glUseProgram(0);
}

void imageRenderer_setTint(imageRenderer* ir, float r, float g, float b)
{
	glUseProgram(ir->program.id);
	glUniform3f(glGetUniformLocation(ir->program.id, "tint"), r, g, b);
	glUseProgram(0);
}

void imageRenderer_setSize(imageRenderer* ir, int width, int height)
{
	glUseProgram(ir->program.id);
	glUniformMatrix4fv(glGetUniformLocation(ir->program.id, "projection"), 1, GL_FALSE, mat4_ortho(0, width, 0, height, -1, 1).data);
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