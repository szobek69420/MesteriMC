#ifndef CHUNK_LOW_RES_H
#define CHUNK_LOW_RES_H

#ifndef CHUNK_WIDTH
#define CHUNK_WIDTH 32
#define CHUNK_HEIGHT 32
#endif

#include "../../mesh/mesh.h"
#include "../../glm2/mat4.h"

struct chunkLowRes {
	int chunkX, chunkY, chunkZ;
	mesh normalMesh;
	mesh waterMesh;
	char isThereNormalMesh;
	char isThereWaterMesh;
	mat4 model;
};
typedef struct chunkLowRes chunkLowRes;

#include "chunkManager.h"


//a meshRaw*-ok feltoltodnek a chunkmesh adataival, amelyek egy kesobbi lepesben be lesznek toltve a video memoriaba
chunkLowRes chunkLowRes_generate(struct chunkManager* cm, int chunkX, int chunkY, int chunkZ, meshRaw* meshNormal, meshRaw* meshWalter);//chunkX: the number of the chunk in the x axis (chunkX=3 means its the 3th (or 4th if we count 0) chunk in the positive x direction)
void chunkLowRes_loadMeshInGPU(chunkLowRes* chomk, meshRaw meshNormal, meshRaw meshWalter);
void chunkLowRes_destroy(struct chunkManager* cm, chunkLowRes* chomk);


void chunkLowRes_drawTerrain(chunkLowRes* chomk);
void chunkLowRes_drawWalter(chunkLowRes* chomk);

void chunkLowRes_getChunkFromPos(vec3 pos, int* chunkX, int* chunkY, int* chunkZ);

void chunkLowRes_resetGenerationInfo();
void chunkLowRes_getGenerationInfo(int* _generated, int* _destroyed);

#endif