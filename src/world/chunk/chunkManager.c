#include "../terrain/FastNoiseLite.h"
#include "chunkManager.h"
#include "chunk.h"
#include "../../utils/list.h"
#include "../../shader/shader.h"

#include <stdlib.h>
#include <stdio.h>

chunkManager chunkManager_create(int seed, int renderDistance)
{
	chunkManager cm;
	cm.seed = seed;
	cm.renderDistance = renderDistance;

	cm.noise= fnlCreateState();
	cm.noise.noise_type = FNL_NOISE_OPENSIMPLEX2;

	cm.pendingUpdates = list_create();
	cm.loadedChunks = list_create();

	return cm;
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

		iterator=list_next(&iterator);
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

		iterator = list_next(&iterator);
	}

	return 0;
}

void chunkManager_searchForUpdates(chunkManager* cm, int playerChunkX, int playerChunkY, int playerChunkZ)
{
	//load
	for (int i = 1; i <= cm->renderDistance; i++)
	{
		for (int x = -i; x <= i; x ++)
		{
			for (int y = -i; y <= i; y ++)
			{
				for (int z = -i; z <= i; z ++)
				{
					if (x != -i && x != i && y != -i && y != i && z != -i && z != i)
						continue;

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

exit_unload:

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
			*chomk=chunk_generate(ceu->chunkX, ceu->chunkY, ceu->chunkZ);
			list_push_back(&(cm->loadedChunks), (void*)chomk);
			break;

		case CHUNKMANAGER_UNLOAD_CHUNK:

			break;


		case CHUNKMANAGER_RELOAD_CHUNK:

			break;
	}

	free(ceu);
}


void chunkManager_drawTerrain(chunkManager* cm, shader* shit, camera* cum, mat4* projection)
{
	shader_setMat4(shit->id, "view", camera_get_view_matrix(cum));
	shader_setMat4(shit->id, "projection", *projection);

	listElement* it = list_get_iterator(&(cm->loadedChunks));
	while (it != NULL)
	{
		//printf("%d %d %d\n", ((chunk*)(it->data))->chunkX, ((chunk*)(it->data))->chunkY, ((chunk*)(it->data))->chunkZ);
		shader_setMat4(shit->id, "model", ((chunk*)(it->data))->model);
		chunk_drawTerrain((chunk*)(it->data));

		//it=it->next;
		it = list_next(&it);
	}

	/*chunk* chomk;
	for (int i = 0; i < cm->loadedChunks.size; i++)
	{
		chomk = list_get(&(cm->loadedChunks), i);
		shader_setMat4(shit->id, "model", chomk->model);
		chunk_drawTerrain(chomk);
	}*/
}

