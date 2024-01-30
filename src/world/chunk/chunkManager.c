#include "chunkManager.h"
#include "chunk.h"
#include "../terrain/FastNoiseLite.h"
#include "../../utils/list.h"
#include "../../utils/lista.h"
#include "../../shader/shader.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <glad/glad.h>


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

	lista_init(cm.pendingUpdates);
	lista_init(cm.pendingMeshUpdates);
	lista_init(cm.loadedChunks);
	
	return cm;
}

void chunkManager_destroy(chunkManager* cm)
{
	chunk chomk;
	chunkGenerationUpdate chomkUp;
	chunkMeshUpdate chomkDown;

	while (cm->loadedChunks.size>0)
	{
		lista_at(cm->loadedChunks, 0, &chomk);

		lista_remove_at(cm->loadedChunks, 0);

		chunk_destroy(&chomk);
	}

	lista_clear(cm->pendingUpdates);

	while (cm->pendingMeshUpdates.size>0)
	{
		lista_at(cm->pendingMeshUpdates, 0, &chomkDown);

		lista_remove_at(cm->pendingMeshUpdates, 0);

		switch (chomkDown.type)
		{
		case CHUNKMANAGER_LOAD_CHUNK:
			if (chomkDown.meshNormal.indexCount != 0)
			{
				free(chomkDown.meshNormal.vertices);
				free(chomkDown.meshNormal.indices);
			}
			if (chomkDown.meshWalter.indexCount != 0)
			{
				free(chomkDown.meshWalter.vertices);
				free(chomkDown.meshWalter.indices);
			}
			chunk_destroy(&chomkDown.chomk);
			break;
		}
	}
}

int chunkManager_isChunkLoaded(chunkManager* cm, int chunkX, int chunkY, int chunkZ)
{
	lista_element_of(chunk)* iterator = cm->loadedChunks.head;

	while (iterator!=NULL)
	{
		if (iterator->data.chunkX == chunkX && iterator->data.chunkY ==chunkY && iterator->data.chunkZ == chunkZ)
			return 69;

		iterator=iterator->next;
	}

	return 0;
}

int chunkManager_isChunkPending(chunkManager* cm, int chunkX, int chunkY, int chunkZ)
{
	lista_element_of(chunkMeshUpdate)* iterator=cm->pendingMeshUpdates.head;
	lista_element_of(chunkGenerationUpdate)* iterator2 = cm->pendingUpdates.head;

	while (iterator2 != NULL)
	{
		if (iterator2->data.chunkX == chunkX && iterator2->data.chunkY == chunkY && iterator2->data.chunkZ == chunkZ)
			return 69;

		iterator2 = iterator2->next;
	}

	while (iterator != NULL)
	{
		if (iterator->data.chomk.chunkX == chunkX && iterator->data.chomk.chunkY == chunkY && iterator->data.chomk.chunkZ == chunkZ)
			return 69;

		iterator = iterator->next;
	}

	return 0;
}

void chunkManager_searchForUpdates(chunkManager* cm, int playerChunkX, int playerChunkY, int playerChunkZ)
{
	lista_element_of(chunk)* iterator;

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
						chunkGenerationUpdate ceu;
						ceu.chunkX = playerChunkX + x;
						ceu.chunkY = playerChunkY + y;
						ceu.chunkZ = playerChunkZ + z;
						ceu.type = CHUNKMANAGER_LOAD_CHUNK;

						lista_push_back(cm->pendingUpdates, ceu);
						goto exit_load;
					}
				}
			}
		}
	}

exit_load:

	//unload
	iterator = cm->loadedChunks.head;
	while (iterator != NULL)
	{
		if (abs(iterator->data.chunkX - playerChunkX) > cm->renderDistance
			||
			abs(iterator->data.chunkY - playerChunkY) > cm->renderDistance
			||
			abs(iterator->data.chunkZ - playerChunkZ) > cm->renderDistance)
		{
			if (chunkManager_isChunkPending(cm, iterator->data.chunkX, iterator->data.chunkY, iterator->data.chunkZ)==0)
			{
				chunkGenerationUpdate ceu;
				ceu.chunkX = iterator->data.chunkX;
				ceu.chunkY = iterator->data.chunkY;
				ceu.chunkZ = iterator->data.chunkZ;
				ceu.type = CHUNKMANAGER_UNLOAD_CHUNK;

				lista_push_back(cm->pendingUpdates, ceu);

				break;
			}
		}

		iterator = iterator->next;
	}


exit_unload:

	return;
}

void chunkManager_update(chunkManager* cm, pthread_mutex_t* pmutex)
{
	lista_element_of(chunk)* iterator;
	int index;
	chunk chomk;
	chunkMeshUpdate cmu;

	pthread_mutex_lock(pmutex);
	if (cm->pendingUpdates.size == 0)
	{
		pthread_mutex_unlock(pmutex);
		return;
	}
	chunkGenerationUpdate ceu;
	lista_at(cm->pendingUpdates, 0, &ceu);
	lista_remove_at(cm->pendingUpdates, 0);
	pthread_mutex_unlock(pmutex);



	switch (ceu.type)
	{
		case CHUNKMANAGER_LOAD_CHUNK:
			cmu.chomk=chunk_generate(cm, ceu.chunkX, ceu.chunkY, ceu.chunkZ, &cmu.meshNormal, &cmu.meshWalter);
			cmu.type = CHUNKMANAGER_LOAD_CHUNK;
			pthread_mutex_lock(pmutex);
			lista_push_back(cm->pendingMeshUpdates, cmu);
			pthread_mutex_unlock(pmutex);
			break;

		case CHUNKMANAGER_UNLOAD_CHUNK:
			pthread_mutex_lock(pmutex);
			iterator = cm->loadedChunks.head;
			index = 0;
			while (iterator != NULL)
			{
				chomk = iterator->data;
				if (chomk.chunkX == ceu.chunkX && chomk.chunkY == ceu.chunkY && chomk.chunkZ == ceu.chunkZ)
				{
					cmu.chomk = chomk;
					cmu.type = CHUNKMANAGER_UNLOAD_CHUNK;
					lista_push_back(cm->pendingMeshUpdates, cmu);

					lista_remove_at(cm->loadedChunks, index);
					break;
				}

				iterator = iterator->next;
				index++;
			}
			pthread_mutex_unlock(pmutex);
			break;


		case CHUNKMANAGER_RELOAD_CHUNK:
			//unload and load
			break;
	}
}

void chunkManager_updateMesh(chunkManager* cm)
{
	if (cm->pendingMeshUpdates.size == 0)
		return;

	lista_element_of(chunkMeshUpdate)* iterator;
	int index;

	chunkMeshUpdate cmu;

	lista_at(cm->pendingMeshUpdates, 0, &cmu);

	lista_remove_at(cm->pendingMeshUpdates, 0);

	switch (cmu.type)
	{
	case CHUNKMANAGER_LOAD_CHUNK:
		chunk_loadMeshInGPU(&cmu.chomk, cmu.meshNormal, cmu.meshWalter);
		lista_push_back(cm->loadedChunks, cmu.chomk);

		if (cmu.meshNormal.indexCount != 0)
		{
			free(cmu.meshNormal.vertices);
			free(cmu.meshNormal.indices);
		}
		if (cmu.meshWalter.indexCount != 0)
		{
			free(cmu.meshWalter.vertices);
			free(cmu.meshWalter.indices);
		}
		break;

	case CHUNKMANAGER_UNLOAD_CHUNK:
		chunk_destroy(&cmu.chomk);
		break;


	case CHUNKMANAGER_RELOAD_CHUNK:
		break;
	}
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

	lista_element_of(chunk)* it = cm->loadedChunks.head;
	while (it != NULL)
	{
		if (it->data.isThereNormalMesh)
		{
			basedX = it->data.chunkX * CHUNK_WIDTH;
			basedY = it->data.chunkY * CHUNK_HEIGHT;
			basedZ = it->data.chunkZ * CHUNK_WIDTH;

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
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, it->data.model.data);
					chunk_drawTerrain(&it->data);
					break;
				}
			}

			if (!isPointInFrustum)//ha nincs pont benne a kamera frustumban
			{
				if (((frustumX[0] && frustumX[2]) || frustumX[1]) &&
					((frustumY[0] && frustumY[2]) || frustumY[1]) &&
					((frustumZ[0] && frustumZ[2]) || frustumZ[1]))
				{
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, it->data.model.data);
					chunk_drawTerrain(&it->data);
				}
			}
		}
		it=it->next;
	}
}

void chunkManager_drawWalter(chunkManager* cm, shader* shit, camera* cum, mat4* projection)
{
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

	lista_element_of(chunk) * it = cm->loadedChunks.head;
	while (it != NULL)
	{
		if (it->data.isThereNormalMesh)
		{
			basedX = it->data.chunkX * CHUNK_WIDTH;
			basedY = it->data.chunkY * CHUNK_HEIGHT;
			basedZ = it->data.chunkZ * CHUNK_WIDTH;

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
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, it->data.model.data);
					chunk_drawWalter(&it->data);
					break;
				}
			}

			if (!isPointInFrustum)//ha nincs pont benne a kamera frustumban
			{
				if (((frustumX[0] && frustumX[2]) || frustumX[1]) &&
					((frustumY[0] && frustumY[2]) || frustumY[1]) &&
					((frustumZ[0] && frustumZ[2]) || frustumZ[1]))
				{
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, it->data.model.data);
					chunk_drawWalter(&it->data);
				}
			}
		}
		it=it->next;
	}
}

void chunkManager_drawShadow(chunkManager* cm, shader* shit, mat4* viewProjection)
{
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
	
	lista_element_of(chunk)* it = cm->loadedChunks.head;
	while (it != NULL)
	{
		if (it->data.isThereNormalMesh)
		{
			basedX = it->data.chunkX * CHUNK_WIDTH;
			basedY = it->data.chunkY * CHUNK_HEIGHT;
			basedZ = it->data.chunkZ * CHUNK_WIDTH;

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
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, it->data.model.data);
					chunk_drawTerrain(&it->data);
					break;
				}
			}

			if (!isPointInFrustum)//ha nincs pont benne a kamera frustumban
			{
				if (((frustumX[0] && frustumX[2]) || frustumX[1]) &&
					((frustumY[0] && frustumY[2]) || frustumY[1]) &&
					((frustumZ[0] && frustumZ[2]) || frustumZ[1]))
				{
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, it->data.model.data);
					chunk_drawTerrain(&it->data);
				}
			}
		}
		it=it->next;
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

