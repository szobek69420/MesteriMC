#include "../terrain/FastNoiseLite.h"
#include "chunkManager.h"
#include "chunk.h"
#include "../../utils/list.h"
#include "../../shader/shader.h"

#include <stdlib.h>
#include <stdio.h>

#include <glad/glad.h>

chunkManager chunkManager_create(int seed, int renderDistance)
{
	chunkManager cm;
	cm.seed = seed;
	cm.renderDistance = renderDistance;

	cm.noise= fnlCreateState();
	cm.noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
	cm.noise.frequency = 0.0017;
	cm.noise.seed = seed;

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

int chunkManager_isChunkLoaded(chunkManager* cm, int chunkX, int chunkY, int chunkZ)
{
	listElement* iterator = list_get_iterator(&(cm->loadedChunks));

	while (iterator!=NULL)
	{
		if (((chunk*)(iterator->data))->chunkX == chunkX && ((chunk*)(iterator->data))->chunkY ==chunkY && ((chunk*)(iterator->data))->chunkZ == chunkZ)
			return 69;

		iterator=list_next(iterator);
	}

	return 0;
}

int chunkManager_isChunkPending(chunkManager* cm, int chunkX, int chunkY, int chunkZ)
{
	listElement* iterator = list_get_iterator(&(cm->pendingUpdates));

	while (iterator != NULL)
	{
		if (((chunkUpdate*)(iterator->data))->chunkX == chunkX && ((chunkUpdate*)(iterator->data))->chunkY == chunkY && ((chunkUpdate*)(iterator->data))->chunkZ == chunkZ)
			return 69;

		iterator = list_next(iterator);
	}

	return 0;
}

void chunkManager_searchForUpdates(chunkManager* cm, int playerChunkX, int playerChunkY, int playerChunkZ)
{
	listElement* iterator;

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
	iterator = list_get_iterator(&(cm->loadedChunks));
	while (iterator != NULL)
	{
		if (abs(((chunk*)iterator->data)->chunkX - playerChunkX) > cm->renderDistance
			||
			abs(((chunk*)iterator->data)->chunkY - playerChunkY) > cm->renderDistance
			||
			abs(((chunk*)iterator->data)->chunkZ - playerChunkZ) > cm->renderDistance)
		{
			if (chunkManager_isChunkPending(cm, ((chunk*)iterator->data)->chunkX, ((chunk*)iterator->data)->chunkY, ((chunk*)iterator->data)->chunkZ)==0)
			{
				chunkUpdate* ceu = (chunkUpdate*)malloc(sizeof(chunkUpdate));
				ceu->chunkX = ((chunk*)iterator->data)->chunkX;
				ceu->chunkY = ((chunk*)iterator->data)->chunkY;
				ceu->chunkZ = ((chunk*)iterator->data)->chunkZ;
				ceu->type = CHUNKMANAGER_UNLOAD_CHUNK;

				list_push_back(&(cm->pendingUpdates), (void*)ceu);

				break;
			}
		}

		iterator = list_next(iterator);
	}


exit_unload:

	return;
}

void chunkManager_update(chunkManager* cm)
{
	listElement* iterator;
	int index;
	chunk* chomk;
	chunkUpdate* ceu = (chunkUpdate*)list_get(&(cm->pendingUpdates), 0);

	if (ceu == NULL)
		return;

	list_remove_at(&(cm->pendingUpdates), 0);

	switch (ceu->type)
	{
		case CHUNKMANAGER_LOAD_CHUNK:
			chomk= (chunk*)malloc(sizeof(chunk));
			*chomk=chunk_generate(cm, ceu->chunkX, ceu->chunkY, ceu->chunkZ);
			list_push_back(&(cm->loadedChunks), (void*)chomk);
			break;

		case CHUNKMANAGER_UNLOAD_CHUNK:
			iterator = list_get_iterator(&(cm->loadedChunks));
			index = 0;
			while (iterator != NULL)
			{
				chomk = (chunk*)iterator->data;
				if (chomk->chunkX == ceu->chunkX && chomk->chunkY == ceu->chunkY && chomk->chunkZ == ceu->chunkZ)
				{
					//list_remove_at(&(cm->loadedChunks), index);
					list_remove_at(&(cm->loadedChunks), index);
					chunk_destroy(chomk);
					free(chomk);
					break;
				}

				iterator = list_next(iterator);
				index++;
			}
			break;


		case CHUNKMANAGER_RELOAD_CHUNK:

			break;
	}

	free(ceu);
}

static float chunkBounds[24] = {
	0,0,0,
	0,0,CHUNK_WIDTH,
	0,CHUNK_HEIGHT,0,
	0,CHUNK_HEIGHT,CHUNK_WIDTH,
	CHUNK_WIDTH,0,0,
	CHUNK_WIDTH,0,CHUNK_WIDTH,
	CHUNK_WIDTH,CHUNK_HEIGHT,0,
	CHUNK_WIDTH,CHUNK_HEIGHT,CHUNK_WIDTH
};

void chunkManager_drawTerrain(chunkManager* cm, shader* shit, camera* cum, mat4* projection)
{
	chunk* chomk;
	GLuint modelLocation = glGetUniformLocation(shit->id, "model");
	char isInFrustum;
	float basedX, basedY, basedZ;
	vec4 temp;
	mat4 pv = mat4_multiply(*projection, camera_getViewMatrix(cum));

	//shader_setMat4(shit->id, "view", camera_getViewMatrix(cum));
	//shader_setMat4(shit->id, "projection", *projection);

	listElement* it = list_get_iterator(&(cm->loadedChunks));
	while (it != NULL)
	{
		chomk = ((chunk*)it->data);
		if (chomk->isThereNormalMesh)
		{
			basedX = chomk->chunkX * CHUNK_WIDTH;
			basedY = chomk->chunkY * CHUNK_HEIGHT;
			basedZ = chomk->chunkZ * CHUNK_WIDTH;

			for (int i = 0; i < 24; )
			{
				temp.x = basedX + chunkBounds[i++];
				temp.y = basedY + chunkBounds[i++];
				temp.z = basedZ + chunkBounds[i++];
				temp.w = 1;
				temp = vec4_multiplyWithMatrix(pv, temp);
				temp.x /= temp.w; temp.y /= temp.w; temp.z /= temp.w;//perspective division

				if (temp.x * temp.x < 1 || temp.y * temp.y < 1 || temp.z * temp.z < 1)
				{
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, chomk->model.data);
					chunk_drawTerrain(chomk);
					break;
				}
			}
		}

		//it=it->next;
		it = list_next(it);
	}
}

