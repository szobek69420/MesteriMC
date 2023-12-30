#include "../terrain/FastNoiseLite.h"
#include "chunkManager.h"
#include "chunk.h"
#include "../../utils/list.h"

chunkManager chunkManager_create(int seed, int renderDistance)
{
	chunkManager cm;
	cm.seed = seed;
	cm.renderDistance = renderDistance;

	cm.noise= fnlCreateState();
	cm.noise.noise_type = FNL_NOISE_OPENSIMPLEX2;

	cm.pendingUpdates = list_create();
	cm.loadedChunks = list_create();
}

void chunkManager_destroy(chunkManager* cm)
{
	chunk* chomk;

	free(cm->noise);

	while (69)
	{
		chomk = (chunk*)list_get(&(cm->loadedChunks), 0);
		if (chomk == NULL)
			break;

		list_remove_at(&(cm->loadedChunks), 0);

		chunk_destroy(chomk);
	}

	list_clear(&(cm->pendingUpdates));
}

void chunkManager_searchForUpdates(int playerChunkX, int playerChunkY, int playerChunkZ);//adds an Eintrag to the pendingUpdates if needed

void chunkManager_update();//erledigt an update from pendingUpdates