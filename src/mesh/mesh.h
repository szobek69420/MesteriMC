#ifndef MESH_H
#define MESH_H

typedef struct {
	unsigned int vbo;
	unsigned int ebo;
	unsigned int vao;

	unsigned int indexCount;
} mesh;

void mesh_destroy(mesh m);

#endif