#ifndef MESH_H
#define MESH_H

typedef struct {
	unsigned int vbo;
	unsigned int ebo;
	unsigned int vao;
} mesh;

/*
* vec4 (pos.x, pos.y, pos.z, geometry)
* vec4 normal ( normal.x, normal.y, normal.z, uv.x)
* vec4 tangent ( tangent.x, tangent.y, tangent.z, uv.y)
*/

#endif