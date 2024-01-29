#include "text_renderer.h"
#include "../font_handler/font_handler.h"

#include "../../glm2/mat4.h"
#include "../../shader/shader.h"

#include <stdlib.h>
#include <glad/glad.h>

textRenderer textRenderer_create(int width, int height)
{
	textRenderer renderer;
	
	//vao, vbo, ebo
	glGenVertexArrays(1, &renderer.vao);
	glBindVertexArray(renderer.vao);

	glGenBuffers(1, &renderer.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, renderer.vbo);
	glBufferData(GL_ARRAY_BUFFER, 4*sizeof(float), NULL, GL_STATIC_DRAW);

	unsigned int indices[] = { 0, 2, 1, 3, 2, 0 };//0: bal also, 1: jobb also, 2: jobb felso, 3: bal felso
	glGenBuffers(1, &renderer.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE,sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	//shader
	renderer.program = shader_import(
		"../assets/shaders/renderer2D/text/shader_text.vag",
		"../assets/shaders/renderer2D/text/shader_text.fag",
		NULL
	);

	textRenderer_setColour(&renderer, 1, 1, 1);
	textRenderer_setSize(&renderer, width, height);

	return renderer;
}

void textRenderer_destroy(textRenderer* renderer)
{
	glDeleteVertexArrays(1, &renderer->vao);
	glDeleteBuffers(1, &renderer->vbo);

	shader_delete(&renderer->program);
}

void textRenderer_render(textRenderer* renderer, font* f, const char* text, float x, float y, float scale)
{
	glUseProgram(renderer->program.id);
	unsigned int dataLocation = glGetUniformLocation(renderer->program.id, "vertexData");

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(renderer->vao);
	// iterate through all characters
	for (int i = 0; text[i] != '\0'; i++)
	{
		character ch = f->characters[text[i]];

		float xpos = x + ch.bearingX * scale;
		float ypos = y - (ch.height - ch.bearingY) * scale;

		float w = ch.width * scale;
		float h = ch.height * scale;
		// update VBO for each character
		float vertices[16] = {
			xpos,     ypos,       0.0f, 1.0f,
			xpos + w, ypos,       1.0f, 1.0f,
			xpos + w, ypos + h,   1.0f, 0.0f,
			xpos,     ypos + h,   0.0f, 0.0f
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		// update data
		glUniformMatrix4fv(dataLocation, 1, GL_FALSE, vertices);
		// render quad
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void textRenderer_setColour(textRenderer* renderer, float r, float g, float b)
{
	glUseProgram(renderer->program.id);
	glUniform3f(glGetUniformLocation(renderer->program.id, "colour"), r, g, b);
	glUseProgram(0);
}

void textRenderer_setSize(textRenderer* renderer, int width, int height)
{
	glUseProgram(renderer->program.id);
	glUniformMatrix4fv(
		glGetUniformLocation(renderer->program.id, "projection"), 
		1, 
		GL_FALSE, 
		mat4_ortho(0, window_getWidth(), 0, window_getHeight(), -1, 1).data
	);
	glUseProgram(0);
}