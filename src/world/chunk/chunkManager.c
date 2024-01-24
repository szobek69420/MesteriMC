#include "../terrain/FastNoiseLite.h"
#include "chunkManager.h"
#include "chunk.h"
#include "../../utils/list.h"
#include "../../shader/shader.h"

#include <stdlib.h>
#include <stdio.h>

#include <glad/glad.h>

struct chunkMeshUpdate{
	struct chunk chomk;
	meshRaw meshNormal;
	meshRaw meshWalter;
	int type;// load/unload/reload
};

char chunkManager_checkIfInFrustum(vec4* vec, char* frustumX, char* frustumY, char* frustumZ);

chunkManager chunkManager_create(int seed, int renderDistance)
{
	chunkManager cm;
	cm.seed = seed;
	cm.renderDistance = renderDistance;

	cm.noise= fnlCreateState();
	cm.noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
	cm.noise.frequency = 0.0017;
	cm.noise.seed = seed;

	cm.noise2 = fnlCreateState();
	cm.noise2.noise_type = FNL_NOISE_OPENSIMPLEX2;
	cm.noise2.frequency = 0.0083;
	cm.noise2.seed = seed*seed+seed;

	cm.pendingUpdates = list_create();
	cm.pendingMeshUpdates = list_create();
	cm.loadedChunks = list_create();

	return cm;
}

void chunkManager_destroy(chunkManager* cm)
{
	chunk* chomk;
	chunkGenerationUpdate* chomkUp;
	chunkMeshUpdate* chomkDown;

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
		chomkUp = (chunkGenerationUpdate*)list_get(&(cm->pendingUpdates), 0);
		if (chomkUp == NULL)
			break;

		list_remove_at(&(cm->pendingUpdates), 0);

		free(chomkUp);
	}

	while (69)
	{
		chomkDown = (chunkMeshUpdate*)list_get(&(cm->pendingMeshUpdates), 0);
		if (chomkDown == NULL)
			break;

		list_remove_at(&(cm->pendingMeshUpdates), 0);

		switch (chomkDown->type)
		{
		case CHUNKMANAGER_LOAD_CHUNK:
			if (chomkDown->meshNormal.indexCount != 0)
			{
				free(chomkDown->meshNormal.vertices);
				free(chomkDown->meshNormal.indices);
			}
			if (chomkDown->meshWalter.indexCount != 0)
			{
				free(chomkDown->meshWalter.vertices);
				free(chomkDown->meshWalter.indices);
			}
			chunk_destroy(&chomkDown->chomk);
			break;
		}
		free(chomkDown);
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
		if (((chunkGenerationUpdate*)(iterator->data))->chunkX == chunkX && ((chunkGenerationUpdate*)(iterator->data))->chunkY == chunkY && ((chunkGenerationUpdate*)(iterator->data))->chunkZ == chunkZ)
			return 69;

		iterator = list_next(iterator);
	}

	iterator = list_get_iterator(&(cm->pendingMeshUpdates));
	while (iterator != NULL)
	{
		if (((chunkMeshUpdate*)(iterator->data))->chomk.chunkX == chunkX && ((chunkMeshUpdate*)(iterator->data))->chomk.chunkY == chunkY && ((chunkMeshUpdate*)(iterator->data))->chomk.chunkZ == chunkZ)
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
						chunkGenerationUpdate* ceu = (chunkGenerationUpdate*)malloc(sizeof(chunkGenerationUpdate));
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
				chunkGenerationUpdate* ceu = (chunkGenerationUpdate*)malloc(sizeof(chunkGenerationUpdate));
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
	chunkMeshUpdate* cmu = (chunkMeshUpdate*)malloc(sizeof(chunkMeshUpdate));

	chunkGenerationUpdate* ceu = (chunkGenerationUpdate*)list_get(&(cm->pendingUpdates), 0);
	
	if (ceu == NULL)
		return;

	list_remove_at(&(cm->pendingUpdates), 0);

	switch (ceu->type)
	{
		case CHUNKMANAGER_LOAD_CHUNK:
			cmu->chomk=chunk_generate(cm, ceu->chunkX, ceu->chunkY, ceu->chunkZ, &cmu->meshNormal, &cmu->meshWalter);
			cmu->type = CHUNKMANAGER_LOAD_CHUNK;
			list_push_back(&cm->pendingMeshUpdates, cmu);
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
					cmu->chomk = *chomk;
					cmu->type = CHUNKMANAGER_UNLOAD_CHUNK;
					list_push_back(&cm->pendingMeshUpdates, cmu);

					list_remove_at(&(cm->loadedChunks), index);
					free(chomk);
					break;
				}

				iterator = list_next(iterator);
				index++;
			}
			break;


		case CHUNKMANAGER_RELOAD_CHUNK:
			//unload and load
			break;
	}

	free(ceu);
}

void chunkManager_updateMesh(chunkManager* cm)
{
	listElement* iterator;
	int index;

	chunk* chomk;
	chunkMeshUpdate* cmu = (chunkMeshUpdate*)list_get(&(cm->pendingMeshUpdates), 0);

	if (cmu == NULL)
	{
		return;
	}

	list_remove_at(&(cm->pendingMeshUpdates), 0);

	switch (cmu->type)
	{
	case CHUNKMANAGER_LOAD_CHUNK:
		chunk_loadMeshInGPU(&cmu->chomk, cmu->meshNormal, cmu->meshWalter);
		chomk = (chunk*)malloc(sizeof(chunk));
		*chomk = cmu->chomk;
		list_push_back(&cm->loadedChunks, (void*)chomk);

		if (cmu->meshNormal.indexCount != 0)
		{
			free(cmu->meshNormal.vertices);
			free(cmu->meshNormal.indices);
		}
		if (cmu->meshWalter.indexCount != 0)
		{
			free(cmu->meshWalter.vertices);
			free(cmu->meshWalter.indices);
		}
		break;

	case CHUNKMANAGER_UNLOAD_CHUNK:
		chunk_destroy(&cmu->chomk);
		break;


	case CHUNKMANAGER_RELOAD_CHUNK:
		break;
	}

	free(cmu);
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

	char frustumX[3] = { 0,0,0 };//volt-e olyan bounding point, ami x<-1 vagy -1<=x<=1 vagy x>1
	char frustumY[3] = { 0,0,0 };//volt-e olyan bounding point, ami y<-1 vagy -1<=y<=1 vagy y>1
	char frustumZ[3] = { 0,0,0 };//volt-e olyan bounding point, ami z<0 vagy 0<=z<=1 vagy z>1
	char isPointInFrustum = 0;

	listElement* it = list_get_iterator(&(cm->loadedChunks));
	while (it != NULL)
	{
		chomk = ((chunk*)it->data);
		if (chomk->isThereNormalMesh)
		{
			basedX = chomk->chunkX * CHUNK_WIDTH;
			basedY = chomk->chunkY * CHUNK_HEIGHT;
			basedZ = chomk->chunkZ * CHUNK_WIDTH;

			frustumX[0] = 0;	frustumX[1] = 0;	frustumX[2] = 0;
			frustumY[0] = 0;	frustumY[1] = 0;	frustumY[2] = 0;
			frustumZ[0] = 0;	frustumZ[1] = 0;	frustumZ[2] = 0;

			for (int i = 0; i < 24; )
			{
				temp.x = basedX + chunkBounds[i++];
				temp.y = basedY + chunkBounds[i++];
				temp.z = basedZ + chunkBounds[i++];
				temp.w = 1;
				temp = vec4_multiplyWithMatrix(pv, temp);
				temp.x /= temp.w; temp.y /= temp.w; temp.z /= temp.w;//perspective division
				
				//frustum cull
				isPointInFrustum = chunkManager_checkIfInFrustum(&temp, frustumX, frustumY, frustumZ);

				if (isPointInFrustum)
				{
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, chomk->model.data);
					chunk_drawTerrain(chomk);
					break;
				}
			}

			if (!isPointInFrustum)//ha nincs pont benne a kamera frustumban
			{
				if (((frustumX[0] && frustumX[2]) || frustumX[1]) &&
					((frustumY[0] && frustumY[2]) || frustumY[1]) &&
					((frustumZ[0] && frustumZ[2]) || frustumZ[1]))
				{
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, chomk->model.data);
					chunk_drawTerrain(chomk);
				}
			}
		}
		//it=it->next;
		it = list_next(it);
	}
}

void chunkManager_drawShadow(chunkManager* cm, shader* shit, mat4* viewProjection)
{
	chunk* chomk;
	GLuint modelLocation = glGetUniformLocation(shit->id, "model");
	char isInFrustum;
	float basedX, basedY, basedZ;
	vec4 temp;

	glUniformMatrix4fv(glGetUniformLocation(shit->id, "lightMatrix"), 1, GL_FALSE, viewProjection->data);
	//shader_setMat4(shit->id, "view", camera_getViewMatrix(cum));
	//shader_setMat4(shit->id, "projection", *projection);

	char frustumX[3] = { 0,0,0 };//volt-e olyan bounding point, ami x<-1 vagy -1<=x<=1 vagy x>1
	char frustumY[3] = { 0,0,0 };//volt-e olyan bounding point, ami y<-1 vagy -1<=y<=1 vagy y>1
	char frustumZ[3] = { 0,0,0 };//volt-e olyan bounding point, ami z<0 vagy 0<=z<=1 vagy z>1
	char isPointInFrustum = 0;

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
				temp = vec4_multiplyWithMatrix(*viewProjection, temp);
				temp.x /= temp.w; temp.y /= temp.w; temp.z /= temp.w;//perspective division

				//frustum cull
				isPointInFrustum = chunkManager_checkIfInFrustum(&temp, frustumX, frustumY, frustumZ);

				if (isPointInFrustum)
				{
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, chomk->model.data);
					//chunk_drawTerrain(chomk);
					break;
				}
			}

			if (!isPointInFrustum)//ha nincs pont benne a kamera frustumban
			{
				if (((frustumX[0] && frustumX[2]) || frustumX[1]) &&
					((frustumY[0] && frustumY[2]) || frustumY[1]) &&
					((frustumZ[0] && frustumZ[2]) || frustumZ[1]))
				{
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, chomk->model.data);
					//chunk_drawTerrain(chomk);
				}
			}
		}
		//it=it->next;
		it = list_next(it);
	}
}

char chunkManager_checkIfInFrustum(vec4* point, char* frustumX, char* frustumY, char* frustumZ)//0-t ad vissza, ha nincs benne a pont a frustumban
{
	//frustum cull
	char isPointInFrustum = 0;

	if (point->x < -1)
		frustumX[0] = 69;
	else if (point->x < 1)
	{
		frustumX[1] = 69;
		isPointInFrustum++;
	}
	else
		frustumX[2] = 69;

	if (point->y < -1)
		frustumY[0] = 69;
	else if (point->y < 1)
	{
		frustumY[1] = 69;
		isPointInFrustum++;
	}
	else
		frustumY[2] = 69;

	if (point->z < 0)
		frustumZ[0] = 69;
	else if (point->z < 1)
	{
		frustumZ[1] = 69;
		isPointInFrustum++;
	}
	else
		frustumZ[2] = 69;

	if (isPointInFrustum == 3)
		return 69;
	return 0;
}

