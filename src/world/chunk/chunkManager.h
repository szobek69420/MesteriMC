#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "../../utils/list.h"
#include "../terrain/FastNoiseLite.h"
#include "../../camera/camera.h"
#include "../../glm2/mat4.h"
#include "../../shader/shader.h"

//chunk update types
#define CHUNKMANAGER_LOAD_CHUNK 1
#define CHUNKMANAGER_UNLOAD_CHUNK 2
#define CHUNKMANAGER_RELOAD_CHUNK 3

typedef struct {
	int chunkX, chunkY, chunkZ;
	int type;
} chunkUpdate;

typedef struct {
	int seed;
	int renderDistance;

	list loadedChunks;//list of chunks
	list pendingUpdates;//list of chunk updates

	fnl_state noise;//terrain generation
} chunkManager;


chunkManager chunkManager_create(int seed, int renderDistance);
void chunkManager_destroy(chunkManager* cm);

void chunkManager_searchForUpdates(chunkManager* cm, int playerChunkX, int playerChunkY, int playerChunkZ);//adds an Eintrag to the pendingUpdates if needed

void chunkManager_update(chunkManager* cm);//erledigt an update from pendingUpdates

void chunkManager_drawTerrain(chunkManager* cm, shader* shit, camera* cum, mat4* projection);//a shader csak átmenetileg van átadva
void chunkManager_drawWalter(chunkManager* cm, shader* shit, camera* cum, mat4* projection);

#endif