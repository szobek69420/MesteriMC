#ifndef CHUNK_H
#define CHUNK_H

#include "../../mesh/mesh.h"

#define CHUNK_WIDTH 32
#define CHUNK_HEIGHT 32

#define CHUNK_NORMAL_MESH_VERTEX_SIZE 15

typedef struct {

	int chunkX, chunkY, chunkZ;

	char blocks[CHUNK_HEIGHT][CHUNK_WIDTH][CHUNK_WIDTH];//y,x,z

	/*
	* normal mesh layout:
	* vec3 pos
	* vec3 uv(2), geometry(1)
	* vec3 normal
	* vec3 tangent
	* vec3 bitangent
	*/
	mesh normalMesh;
	
	mesh waterMesh;//walter

	char isThereNormalMesh;
	char isThereWaterMesh;

} chunk;

chunk chunk_generate(int chunkX, int chunkY, int chunkZ);//chunkX: the number of the chunk in the x axis (chunkX=3 means its the 3th (or 4th if we count 0) chunk in the positive x direction)
void chunk_destroy(chunk* chomk);


void chunk_drawTerrain(chunk* chomk);
void chunk_drawWater(chunk* chomk);

#endif