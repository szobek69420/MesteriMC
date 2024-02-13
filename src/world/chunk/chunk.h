#ifndef CHUNK_H
#define CHUNK_H

#ifndef CHUNK_WIDTH
#define CHUNK_WIDTH 32
#define CHUNK_HEIGHT 32
#endif

#include "../../mesh/mesh.h"
#include "../../glm2/mat4.h"

struct chunk {

	int chunkX, chunkY, chunkZ;

	char*** blocks;//[y][x][z]

	/*
	* old normal mesh layout:
	* vec3 pos
	* vec2 uv(2)
	* vec3 normal
	* vec3 tangent
	*/

	/*
	based normal mesh layout:
	uint: [0aaxxxxxxyyyyyyzzzzzzuuuuvvvviii]
	a: ambient occlusion [0...3] -> 2 bits
	x: pos x [0...32] -> 6 bits
	y: pos y [0...32] -> 6 bits
	z: pos z [0...32] -> 6 bits
	u: uv x [0...10] -> 4 bits
	v: uv y [0...10] -> 4 bits
	i: side id [0...5] -> 3 bits

	//actual uv values: uv*0.1
	*/
	mesh normalMesh;
	

	/*
	based walter mesh layout:
	uint: [00000000000xxxxxxyyyyyyzzzzzziii]
	vec2 uv (32 block/1 uv)

	x: pos x [0...32] -> 6 bits
	y: pos y [0...32] -> 6 bits
	z: pos z [0...32] -> 6 bits
	i: side id [0...5] -> 3 bits
	*/
	mesh waterMesh;//walter

	char isThereNormalMesh;
	char isThereWaterMesh;

	mat4 model;

	unsigned int normalColliderGroupId;
	unsigned int waterColliderGroupId;
};

typedef struct chunk chunk;

#include "chunkManager.h"//azert kell a struct chunk definicioja utan, mert kulonben osszeakad a chunkManager.h-val

//a meshRaw*-ok feltoltodnek a chunkmesh adataival, amelyek egy kesobbi lepesben be lesznek toltve a video memoriaba
chunk chunk_generate(struct chunkManager* cm, int chunkX, int chunkY, int chunkZ, meshRaw* meshNormal, meshRaw* meshWalter);//chunkX: the number of the chunk in the x axis (chunkX=3 means its the 3th (or 4th if we count 0) chunk in the positive x direction)
void chunk_loadMeshInGPU(chunk* chomk, meshRaw meshNormal, meshRaw meshWalter);
void chunk_destroy(struct chunkManager* cm, chunk* chomk);


void chunk_drawTerrain(chunk* chomk);
void chunk_drawWalter(chunk* chomk);

void chunk_getChunkFromPos(vec3 pos, int* chunkX, int* chunkY, int* chunkZ);

void chunk_resetGenerationInfo();
void chunk_getGenerationInfo(int* _generated, int* _destroyed);

#endif