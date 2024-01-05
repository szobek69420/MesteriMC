#include "sphere.h"
#include "../mesh.h"
#include <glad/glad.h>

static float vertices[SPHERE_VERTEX_COUNT];
static unsigned int indices[SPHERE_INDEX_COUNT];

mesh sphere_create()
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

	return m;
}

static float vertices[] = {
	0, 1, 0,
	0.50000006, 0.8660254, 0,
	0.43301278, 0.8660254, 0.25,
	0.25000006, 0.8660254, 0.43301275,
	0, 0.8660254, 0.50000006,
	-0.24999996, 0.8660254, 0.4330128,
	-0.4330127, 0.8660254, 0.25000015,
	-0.50000006, 0.8660254, 0,
	-0.43301287, 0.8660254, -0.24999982,
	-0.25000018, 0.8660254, -0.43301266,
	0, 0.8660254, -0.50000006,
	0.24999978, 0.8660254, -0.4330129,
	0.4330126, 0.8660254, -0.25000033,
	0.86602545, 0.49999997, 0,
	0.75000006, 0.49999997, 0.4330127,
	0.43301278, 0.49999997, 0.75,
	0, 0.49999997, 0.86602545,
	-0.4330126, 0.49999997, 0.7500001,
	-0.74999994, 0.49999997, 0.43301293,
	-0.86602545, 0.49999997, 0,
	-0.75000024, 0.49999997, -0.43301237,
	-0.433013, 0.49999997, -0.7499999,
	0, 0.49999997, -0.86602545,
	0.43301228, 0.49999997, -0.7500003,
	0.74999976, 0.49999997, -0.43301323,
	1, 0, 0,
	0.86602545, 0, 0.49999997,
	0.50000006, 0, 0.8660254,
	0, 0, 1,
	-0.49999985, 0, 0.8660255,
	-0.86602527, 0, 0.50000024,
	-1, 0, 0,
	-0.8660256, 0, -0.49999958,
	-0.5000003, 0, -0.8660252,
	0, 0, -1,
	0.4999995, 0, -0.8660257,
	0.8660251, 0, -0.5000006,
	0.86602545, -0.49999997, 0,
	0.75000006, -0.49999997, 0.4330127,
	0.43301278, -0.49999997, 0.75,
	0, -0.49999997, 0.86602545,
	-0.4330126, -0.49999997, 0.7500001,
	-0.74999994, -0.49999997, 0.43301293,
	-0.86602545, -0.49999997, 0,
	-0.75000024, -0.49999997, -0.43301237,
	-0.433013, -0.49999997, -0.7499999,
	0, -0.49999997, -0.86602545,
	0.43301228, -0.49999997, -0.7500003,
	0.74999976, -0.49999997, -0.43301323,
	0.50000006, -0.8660254, 0,
	0.43301278, -0.8660254, 0.25,
	0.25000006, -0.8660254, 0.43301275,
	0, -0.8660254, 0.50000006,
	-0.24999996, -0.8660254, 0.4330128,
	-0.4330127, -0.8660254, 0.25000015,
	-0.50000006, -0.8660254, 0,
	-0.43301287, -0.8660254, -0.24999982,
	-0.25000018, -0.8660254, -0.43301266,
	0, -0.8660254, -0.50000006,
	0.24999978, -0.8660254, -0.4330129,
	0.4330126, -0.8660254, -0.25000033,
	0, -1, 0
};

static unsigned int indices[] = {
	0, 1, 2,
	0, 2, 3,
	0, 3, 4,
	0, 4, 5,
	0, 5, 6,
	0, 6, 7,
	0, 7, 8,
	0, 8, 9,
	0, 9, 10,
	0, 10, 11,
	0, 11, 12,
	0, 12, 1,
	1, 13, 14,
	2, 14, 15,
	3, 15, 16,
	4, 16, 17,
	5, 17, 18,
	6, 18, 19,
	7, 19, 20,
	8, 20, 21,
	9, 21, 22,
	10, 22, 23,
	11, 23, 24,
	12, 24, 13,
	1, 14, 2,
	2, 15, 3,
	3, 16, 4,
	4, 17, 5,
	5, 18, 6,
	6, 19, 7,
	7, 20, 8,
	8, 21, 9,
	9, 22, 10,
	10, 23, 11,
	11, 24, 12,
	12, 13, 1,
	13, 25, 26,
	14, 26, 27,
	15, 27, 28,
	16, 28, 29,
	17, 29, 30,
	18, 30, 31,
	19, 31, 32,
	20, 32, 33,
	21, 33, 34,
	22, 34, 35,
	23, 35, 36,
	24, 36, 25,
	13, 26, 14,
	14, 27, 15,
	15, 28, 16,
	16, 29, 17,
	17, 30, 18,
	18, 31, 19,
	19, 32, 20,
	20, 33, 21,
	21, 34, 22,
	22, 35, 23,
	23, 36, 24,
	24, 25, 13,
	25, 37, 38,
	26, 38, 39,
	27, 39, 40,
	28, 40, 41,
	29, 41, 42,
	30, 42, 43,
	31, 43, 44,
	32, 44, 45,
	33, 45, 46,
	34, 46, 47,
	35, 47, 48,
	36, 48, 37,
	25, 38, 26,
	26, 39, 27,
	27, 40, 28,
	28, 41, 29,
	29, 42, 30,
	30, 43, 31,
	31, 44, 32,
	32, 45, 33,
	33, 46, 34,
	34, 47, 35,
	35, 48, 36,
	36, 37, 25,
	37, 49, 50,
	38, 50, 51,
	39, 51, 52,
	40, 52, 53,
	41, 53, 54,
	42, 54, 55,
	43, 55, 56,
	44, 56, 57,
	45, 57, 58,
	46, 58, 59,
	47, 59, 60,
	48, 60, 49,
	37, 50, 38,
	38, 51, 39,
	39, 52, 40,
	40, 53, 41,
	41, 54, 42,
	42, 55, 43,
	43, 56, 44,
	44, 57, 45,
	45, 58, 46,
	46, 59, 47,
	47, 60, 48,
	48, 49, 37,
	61, 49, 60,
	61, 50, 49,
	61, 51, 50,
	61, 52, 51,
	61, 53, 52,
	61, 54, 53,
	61, 55, 54,
	61, 56, 55,
	61, 57, 56,
	61, 58, 57,
	61, 59, 58,
	61, 60, 59
};