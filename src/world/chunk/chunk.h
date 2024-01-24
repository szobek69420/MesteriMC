#ifndef CHUNK_H
#define CHUNK_H

#include "chunkManager.h"
#include "../../mesh/mesh.h"
#include "../../glm2/mat4.h"

#define CHUNK_WIDTH 32
#define CHUNK_HEIGHT 32

#define CHUNK_NORMAL_MESH_VERTEX_SIZE 11

struct chunk {

	int chunkX, chunkY, chunkZ;

	char*** blocks;//[y][x][z]

	/*
	* normal mesh layout:
	* vec3 pos
	* vec2 uv(2)
	* vec3 normal
	* vec3 tangent
	*/
	mesh normalMesh;
	
	mesh waterMesh;//walter

	char isThereNormalMesh;
	char isThereWaterMesh;

	mat4 model;

};

typedef struct chunk chunk;

chunk chunk_generate(struct chunkManager* cm, int chunkX, int chunkY, int chunkZ, meshRaw* meshNormal, meshRaw* meshWalter);//chunkX: the number of the chunk in the x axis (chunkX=3 means its the 3th (or 4th if we count 0) chunk in the positive x direction)
void chunk_loadMeshInGPU(chunk* chomk, meshRaw meshNormal, meshRaw meshWalter);
void chunk_destroy(chunk* chomk);


void chunk_drawTerrain(chunk* chomk);
void chunk_drawWalter(chunk* chomk);

void chunk_getChunkFromPos(vec3 pos, int* chunkX, int* chunkY, int* chunkZ);

#endif