#include "mesh.h"
#include <glad/glad.h>

void mesh_destroy(mesh m)
{
	glDeleteBuffers(1, &m.vbo);

	if (m.ebo != 0)
		glDeleteBuffers(1, &m.ebo);

	glDeleteVertexArrays(1, &m.vao);
}