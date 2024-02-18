#include "mesh_ui_renderer.h"

#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../shader/shader.h"
#include "../../glm2/mat4.h"
#include "../../mesh/mesh.h"

struct meshUiRenderer {
	shader program;
};

meshUiRenderer* meshUiRenderer_create(int width, int height)
{
	meshUiRenderer* amogus;
	amogus = malloc(sizeof(meshUiRenderer));

	amogus->program = shader_import(
		"../assets/shaders/renderer2D/mesh_ui/shader_mesh_ui.vag",
		"../assets/shaders/renderer2D/mesh_ui/shader_mesh_ui.fag",
		NULL
	);

	glUseProgram(amogus->program.id);
	glUniformMatrix4fv(glGetUniformLocation(amogus->program.id, "projection"), 1, GL_FALSE, mat4_ortho(0, width, 0, height, -1, 1).data);
	glUniform1i(glGetUniformLocation(amogus->program.id, "tex"), 0);
	glUseProgram(0);

	return amogus;
}

void meshUiRenderer_destroy(meshUiRenderer* mur)
{
	glDeleteProgram(mur->program.id);
	mur->program.id = 0;
	free(mur);
}

void meshUiRenderer_render(meshUiRenderer* mur, mesh* m, unsigned int texture, float posX, float posY, float scaleX, float scaleY)
{
	glUseProgram(mur->program.id);
	glUniform4f(glGetUniformLocation(mur->program.id, "pos_scale"), posX, posY, scaleX, scaleY);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(m->vao);

	glDrawElements(GL_TRIANGLES, m->indexCount, GL_UNSIGNED_INT, NULL);

	glBindVertexArray(0);
	glUseProgram(0);
}

void meshUiRenderer_setSize(meshUiRenderer* mur, int width, int height)
{
	glUseProgram(mur->program.id);
	glUniformMatrix4fv(glGetUniformLocation(mur->program.id, "projection"), 1, GL_FALSE, mat4_ortho(0, width, 0, height, -1, 1).data);
	glUseProgram(0);
}