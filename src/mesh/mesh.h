#ifndef MESH_H
#define MESH_H

typedef struct {
	unsigned int vbo;
	unsigned int ebo;
	unsigned int vao;

	unsigned int indexCount;
} mesh;

typedef struct {
	float* vertices;
	unsigned int* indices;
	unsigned int sizeVertices;
	unsigned int sizeIndices;
	unsigned int indexCount;
}meshRaw;

void mesh_destroy(mesh m);

#endif