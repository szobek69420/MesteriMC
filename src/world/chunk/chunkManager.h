#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "../../utils/list.h"
#include "../terrain/FastNoiseLite.h"
#include "../../camera/camera.h"
#include "../../glm2/mat4.h"
#include "../../shader/shader.h"

#include "../../mesh/mesh.h"
#include "chunk.h"

#include <pthread.h>

/*
chunk load :

[GENERATION THREAD] add chunkGenerationUpdate to pendingUpdates
[GENERATION THREAD] generate vertex data AND remove chunkGenerationUpdate from pendingUpdates AND add chunkGenerationUpdate to pendingMeshUpdates
[RENDER THREAD] load vertex data in gpu on render thread AND add chunk to loadedChunks


chunk unload :

[GENERATION THREAD] add chunkGenerationUpdate to pendingUpdates AND remove chunk from loadedChunks
[GENERATION THREAD] add chunkMeshUpdate to pendingMeshUpdates
[RENDER THREAD] delete vertex data from gpu
*/

//chunk update types
#define CHUNKMANAGER_LOAD_CHUNK 1
#define CHUNKMANAGER_UNLOAD_CHUNK 2
#define CHUNKMANAGER_RELOAD_CHUNK 3

typedef struct {
	int chunkX, chunkY, chunkZ;
	int type;
} chunkGenerationUpdate;

struct chunkMeshUpdate;
typedef struct chunkMeshUpdate chunkMeshUpdate;


typedef struct chunkManager {
	int seed;
	int renderDistance;

	list loadedChunks;//list of chunks
	list pendingUpdates;//list of chunk updates
	list pendingMeshUpdates;//list of chunk mesh updates

	fnl_state noise, noise2;//terrain generation
} chunkManager;


chunkManager chunkManager_create(int seed, int renderDistance);
void chunkManager_destroy(chunkManager* cm);

void chunkManager_searchForUpdates(chunkManager* cm, int playerChunkX, int playerChunkY, int playerChunkZ);//adds an Eintrag to the pendingUpdates if needed

void chunkManager_update(chunkManager* cm, pthread_mutex_t* pmutex);//erledigt an update from pendingUpdates (azert kell a mutex, hogy a chunk generalas kozben ne legyen lezarva, csupan amikor hozzaadja a pendingMeshUpdates-hoz)
void chunkManager_updateMesh(chunkManager* cm);

void chunkManager_drawTerrain(chunkManager* cm, shader* shit, camera* cum, mat4* projection);//a shader csak átmenetileg van átadva
void chunkManager_drawWalter(chunkManager* cm, shader* shit, camera* cum, mat4* projection);

void chunkManager_drawShadow(chunkManager* cm, shader* shit, mat4* viewProjection);

#endif