#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "../../utils/list.h"
#include "../../utils/lista.h"
#include "../../utils/seqtor.h"

#include "../terrain/FastNoiseLite.h"

#include "../../camera/camera.h"
#include "../../glm2/mat4.h"
#include "../../shader/shader.h"

#include "../../mesh/mesh.h"
#include "chunk.h"
#include "../blocks/blocks.h"

#include "../../physics/physics_system/physics_system.h"

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

struct chunkGenerationUpdate{
	int chunkX, chunkY, chunkZ;
	int type;
};
typedef struct chunkGenerationUpdate chunkGenerationUpdate;

struct chunkMeshUpdate {
	struct chunk chomk;
	meshRaw meshNormal;
	meshRaw meshWalter;
	int type;// load/unload/reload
};
typedef struct chunkMeshUpdate chunkMeshUpdate;

struct changedBlocksInChunk {//egy chunkhoz tartozo valtozott blokkok
	int chunkX, chunkY, chunkZ, isRegistered;//isRegistered: ha ez 0, akkor az azt jelenti, hogy az adott chunk listajat valamelyik kornyezo chunk toltotte be, azaz az o tartalma meg nincs benne (pl a fak hianyoznak)
	seqtor_of(blockModel) blocks;
};
typedef struct changedBlocksInChunk changedBlocksInChunk;

struct chunkManager {
	int seed;
	int renderDistance;

	lista_of(chunk) loadedChunks;//list of chunks
	lista_of(chunkGenerationUpdate) pendingUpdates;//list of chunk updates
	lista_of(chunkMeshUpdate) pendingMeshUpdates;//list of chunk mesh updates

	seqtor_of(changedBlocksInChunk) changedBlocks;//a kulso vektor chunkonkent osztja fel

	fnl_state noise, noise2;//terrain generation

	physicsSystem* ps;
};
typedef struct chunkManager chunkManager;


chunkManager chunkManager_create(int seed, int renderDistance, physicsSystem* ps);
void chunkManager_destroy(chunkManager* cm);

void chunkManager_searchForUpdates(chunkManager* cm, int playerChunkX, int playerChunkY, int playerChunkZ);//adds an Eintrag to the pendingUpdates if needed

void chunkManager_update(chunkManager* cm, pthread_mutex_t* pmutex);//erledigt an update from pendingUpdates (azert kell a mutex, hogy a chunk generalas kozben ne legyen lezarva, csupan amikor hozzaadja a pendingMeshUpdates-hoz)
void chunkManager_updateMesh(chunkManager* cm);

void chunkManager_reloadChunk(chunkManager* cm, pthread_mutex_t* pmutex, int chunkX, int chunkY, int chunkZ);

int chunkManager_drawTerrain(chunkManager* cm, shader* shit, camera* cum, mat4* projection);//a shader csak átmenetileg van átadva
void chunkManager_drawWalter(chunkManager* cm, shader* shit, camera* cum, mat4* projection);

void chunkManager_drawShadow(chunkManager* cm, shader* shit, mat4* viewProjection);

void chunkManager_changeBlock(chunkManager* cm, int chunkX, int chunkY, int chunkZ, int x, int y, int z, int type);

#endif