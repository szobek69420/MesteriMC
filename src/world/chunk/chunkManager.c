#include "../terrain/FastNoiseLite.h"
#include "chunkManager.h"
#include "chunk.h"
#include "../../utils/list.h"

#include <stdlib.h>

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
	chunkUpdate* chomkUp;

	while (69)
	{
		chomk = (chunk*)list_get(&(cm->loadedChunks), 0);
		if (chomk == NULL)
			break;

		list_remove_at(&(cm->loadedChunks), 0);

		chunk_destroy(chomk);
		free(chomk);
	}

	while (69)
	{
		chomkUp = (chunkUpdate*)list_get(&(cm->pendingUpdates), 0);
		if (chomkUp == NULL)
			break;

		list_remove_at(&(cm->pendingUpdates), 0);

		chunk_destroy(chomkUp);
		free(chomkUp);
	}
}

int chunkManager_isChunkLoaded(chunkManager* cm, int playerChunkX, int playerChunkY, int playerChunkZ)
{
	listElement* iterator = list_get_iterator(&(cm->loadedChunks));

	while (iterator!=NULL)
	{
		if (((chunk*)(iterator->data))->chunkX == playerChunkX && ((chunk*)(iterator->data))->chunkY == playerChunkY && ((chunk*)(iterator->data))->chunkZ == playerChunkZ)
			return 69;

		iterator=list_next(iterator);
	}

	return 0;
}

int chunkManager_isChunkPending(chunkManager* cm, int playerChunkX, int playerChunkY, int playerChunkZ)
{
	listElement* iterator = list_get_iterator(&(cm->pendingUpdates));

	while (iterator != NULL)
	{
		if (((chunkUpdate*)(iterator->data))->chunkX == playerChunkX && ((chunkUpdate*)(iterator->data))->chunkY == playerChunkY && ((chunkUpdate*)(iterator->data))->chunkZ == playerChunkZ)
			return 69;

		iterator = list_next(iterator);
	}

	return 0;
}

void chunkManager_searchForUpdates(chunkManager* cm, int playerChunkX, int playerChunkY, int playerChunkZ)
{
	//load
	for (int i = 1; i <= cm->renderDistance; i++)
	{
		for (int x = -i; x <= i; x += 2 * i)
		{
			for (int y = -i; y <= i; y += 2 * i)
			{
				for (int z = -i; z <= i; z += 2 * i)
				{
					if (chunkManager_isChunkLoaded(cm, playerChunkX + x, playerChunkY + y, playerChunkZ + z) == 0
						&&
						chunkManager_isChunkPending(cm, playerChunkX + x, playerChunkY + y, playerChunkZ + z) == 0)
					{
						chunkUpdate* ceu = (chunkUpdate*)malloc(sizeof(chunkUpdate));
						ceu->chunkX = playerChunkX + x;
						ceu->chunkY = playerChunkY + y;
						ceu->chunkZ = playerChunkZ + z;
						ceu->type = CHUNKMANAGER_LOAD_CHUNK;

						list_push_back(&(cm->pendingUpdates), (void*)ceu);
						goto exit_load;
					}
				}
			}
		}
	}

exit_load:

	//unload

	return;
}

void chunkManager_update(chunkManager* cm)
{
	chunk* chomk;
	chunkUpdate* ceu = (chunkUpdate*)list_get(&(cm->pendingUpdates), 0);

	if (ceu == NULL)
		return;

	list_remove_at(&(cm->pendingUpdates), 0);

	switch (ceu->type)
	{
		case CHUNKMANAGER_LOAD_CHUNK:
			chomk= (chunk*)malloc(sizeof(chunk));
			chunk_generate(ceu->chunkX, ceu->chunkY, ceu->chunkZ);
			list_push_back(&(cm->loadedChunks), (void*)chomk);
			break;

		case CHUNKMANAGER_UNLOAD_CHUNK:

			break;


		case CHUNKMANAGER_RELOAD_CHUNK:

			break;
	}

	free(ceu);
}

