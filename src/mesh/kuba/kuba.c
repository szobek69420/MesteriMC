#include "kuba.h"
#include "../mesh.h"

#include <glad/glad.h>


static float vertices[KUBA_VERTEX_VALUE_COUNT];
static unsigned int indices[KUBA_INDEX_COUNT];

mesh kuba_create()
{
	mesh m;

	glGenVertexArrays(1, &m.vao);
	glBindVertexArray(m.vao);

	glGenBuffers(1, &m.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &m.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	m.indexCount = KUBA_INDEX_COUNT;

	return m;
}

static float vertices[] = {
	-1, 1, 1,
	1, 1, 1,
	1, 1, -1,
	-1, 1, -1,
	-1, -1, 1,
	1, -1, 1,
	1, -1, -1,
	-1, -1, -1
};

static unsigned int indices[] = {
	0, 1, 2,
	0, 2, 3,
	0, 5, 1,
	0, 4, 5,
	1, 6, 2,
	1, 5, 6,
	2, 6, 7,
	2, 7, 3,
	3, 7, 0,
	7, 4, 0,
	5, 7, 6,
	5, 4, 7
};