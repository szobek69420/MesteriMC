#include "chunk.h"
#include "chunkManager.h"
#include "../terrain/FastNoiseLite.h"
#include "../../utils/list.h"
#include "../../utils/lista.h"
#include "../../shader/shader.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <glad/glad.h>


char chunkManager_checkIfInFrustum(vec4* vec, char* frustumX, char* frustumY, char* frustumZ);
int chunkManager_isChunkRegistered(chunkManager* cm, int chunkX, int chunkY, int chunkZ);//elso generalaskor a chunknak foglalni kell helyet a changedBlocks vektorban

chunkManager chunkManager_create(int seed, int renderDistance, physicsSystem* ps)
{
	chunkManager cm;
	cm.seed = seed;
	cm.renderDistance = renderDistance;

	cm.ps = ps;

	cm.noise= fnlCreateState();
	cm.noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
	cm.noise.frequency = CHUNKMANAGER_TERRAIN_GENERATION_FREQUENCY_1;
	cm.noise.seed = seed;

	cm.noise2 = fnlCreateState();
	cm.noise2.noise_type = FNL_NOISE_OPENSIMPLEX2;
	cm.noise2.frequency = CHUNKMANAGER_TERRAIN_GENERATION_FREQUENCY_2;
	cm.noise2.seed = seed*seed+seed;

	lista_init(cm.pendingUpdates);
	lista_init(cm.pendingMeshUpdates);
	seqtor_init(cm.loadedChunks,1);
	seqtor_init(cm.isChunkCulled, 1);

	seqtor_init(cm.changedBlocks, 1);

	cm.chunkUpdateHelper = malloc((2 * renderDistance + 1) * (2 * renderDistance + 1) * (2 * renderDistance + 1) * sizeof(char));
	
	return cm;
}

void chunkManager_destroy(chunkManager* cm)
{
	chunkMeshUpdate chomkDown;

	for(int i=0;i<seqtor_size(cm->loadedChunks);i++)
	{
		chunk_destroy(cm, &seqtor_at(cm->loadedChunks, i));
	}
	seqtor_destroy(cm->loadedChunks);

	seqtor_destroy(cm->isChunkCulled);

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
			chunk_destroy(cm, &chomkDown.chomk);
			break;

		case CHUNKMANAGER_UNLOAD_CHUNK:
			chunk_destroy(cm, &chomkDown.chomk);
			break;
		}
	}

	for (int i = 0; i < cm->changedBlocks.size; i++)//chunkonkenti vektorok kiuritese
		seqtor_destroy(seqtor_at(cm->changedBlocks, i).blocks);
	seqtor_destroy(cm->changedBlocks);//a kulso vektor kiuritese


	free(cm->chunkUpdateHelper);
}

int chunkManager_isChunkLoaded(chunkManager* cm, int chunkX, int chunkY, int chunkZ)
{
	for(int i=0;i<seqtor_size(cm->loadedChunks);i++)
	{
		if (seqtor_at(cm->loadedChunks,i).chunkX == chunkX && seqtor_at(cm->loadedChunks, i).chunkY ==chunkY && seqtor_at(cm->loadedChunks, i).chunkZ == chunkZ)
			return 69;
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

int chunkManager_isChunkRegistered(chunkManager* cm, int chunkX, int chunkY, int chunkZ)
{
	for (int i = 0; i < cm->changedBlocks.size; i++)
	{
		if (seqtor_at(cm->changedBlocks, i).chunkX != chunkX)
			continue;
		if (seqtor_at(cm->changedBlocks, i).chunkY != chunkY)
			continue;
		if (seqtor_at(cm->changedBlocks, i).chunkZ != chunkZ)
			continue;

		return seqtor_at(cm->changedBlocks, i).isRegistered;
	}

	return 0;
}


int chunkManager_searchForUpdates(chunkManager* cm, int playerChunkX, int playerChunkY, int playerChunkZ)//return 0 if no update has been found
{
	/*
	* the chunkmanager object has a helper array of type char and size (2*renderDistance+1)^3 bytes
	* this is exactly so many members as there should be chunks loaded with a render distance of renderDistance
	* each member (a char variable) has the value 0 if the corresponding chunk is not loaded, otherwise 69 (or something)
	* the indexing of the array works so:
	* let the zero chunk coordinate be (playerChunkX; playerChunkY; playerChunkZ)-(renderDistance;renderDistance;renderDistance)
	* in this coordinate system, to get, whether a chunk is loaded or not, we check like so:
	* chunkUpdateHelper[chunkX*(2*renderDistance+1)^2+chunkY*(2*renderDistance+1)+chunkZ]!=0
	* 
	* we iterate through the loaded chunks and the pending updates, filling up the helper array
	* we then choose the unloaded chunk closest to the player to load
	* while we iterate through the loaded chunks, we can find a chunk that has to be unloaded
	*/

	int updateFound = 0;
	memset(cm->chunkUpdateHelper, 0, (2 * cm->renderDistance + 1) * (2 * cm->renderDistance + 1) * (2 * cm->renderDistance + 1) * sizeof(char));

	int unload = -1;
	int helper = 2 * cm->renderDistance+1;
	int helper2 = helper / 2;
	for (int i = 0; i < seqtor_size(cm->loadedChunks); i++)
	{
		int x, y, z;
		x = seqtor_at(cm->loadedChunks, i).chunkX - playerChunkX + cm->renderDistance;
		y = seqtor_at(cm->loadedChunks, i).chunkY - playerChunkY + cm->renderDistance;
		z = seqtor_at(cm->loadedChunks, i).chunkZ - playerChunkZ + cm->renderDistance;

		if (x<0 || x>=helper || y<0 || y>=helper || z<0 || z>=helper)
		{
			if (unload == -1)
			{
				if (chunkManager_isChunkPending(cm, seqtor_at(cm->loadedChunks, i).chunkX, seqtor_at(cm->loadedChunks, i).chunkY, seqtor_at(cm->loadedChunks, i).chunkZ) == 0)
				{
					unload = i;
				}
			}

			continue;
		}

		cm->chunkUpdateHelper[x * helper * helper + y * helper + z] = 69;
	}

	for (lista_element_of(chunkGenerationUpdate)* it = cm->pendingUpdates.head; it != NULL; it = it->next)
	{
		int x, y, z;
		x = it->data.chunkX - playerChunkX + cm->renderDistance;
		y = it->data.chunkY - playerChunkY + cm->renderDistance;
		z = it->data.chunkZ - playerChunkZ + cm->renderDistance;

		if (x < 0 || x >= helper || y < 0 || y >= helper || z < 0 || z >= helper)
			continue;

		cm->chunkUpdateHelper[x * helper * helper + y * helper + z] = 69;
	}

	for (lista_element_of(chunkMeshUpdate)* it = cm->pendingMeshUpdates.head; it != NULL; it = it->next)
	{
		int x, y, z;
		x = it->data.chomk.chunkX - playerChunkX + cm->renderDistance;
		y = it->data.chomk.chunkY - playerChunkY + cm->renderDistance;
		z = it->data.chomk.chunkZ - playerChunkZ + cm->renderDistance;

		if (x < 0 || x >= helper || y < 0 || y >= helper || z < 0 || z >= helper)
			continue;

		cm->chunkUpdateHelper[x * helper * helper + y * helper + z] = 69;
	}

	
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

					if (cm->chunkUpdateHelper[(x+helper2)*helper*helper+(y+helper2)*helper+(z+helper2)]==0)
					{
						chunkGenerationUpdate ceu;
						ceu.chunkX = playerChunkX + x;
						ceu.chunkY = playerChunkY + y;
						ceu.chunkZ = playerChunkZ + z;
						ceu.type = CHUNKMANAGER_LOAD_CHUNK;
						ceu.isUrgent = 0;

						lista_push_back(cm->pendingUpdates, ceu);
						updateFound = 1;
						goto exit_load;
					}
				}
			}
		}
	}

exit_load:

	//unload
	if (unload != -1)
	{
		chunkGenerationUpdate ceu;
		ceu.chunkX = seqtor_at(cm->loadedChunks, unload).chunkX;
		ceu.chunkY = seqtor_at(cm->loadedChunks, unload).chunkY;
		ceu.chunkZ = seqtor_at(cm->loadedChunks, unload).chunkZ;
		ceu.type = CHUNKMANAGER_UNLOAD_CHUNK;
		ceu.isUrgent = 0;

		lista_push_back(cm->pendingUpdates, ceu);
		updateFound = 1;
	}


exit_unload:

	return updateFound;
}

void chunkManager_update(chunkManager* cm, pthread_mutex_t* pmutex)
{
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
			if(ceu.isUrgent!=0)
				lista_push(cm->pendingMeshUpdates, 0, cmu);
			else
				lista_push_back(cm->pendingMeshUpdates, cmu);
			pthread_mutex_unlock(pmutex);
			break;

		case CHUNKMANAGER_UNLOAD_CHUNK:
			pthread_mutex_lock(pmutex);
			for(int i=0;i<seqtor_size(cm->loadedChunks);i++)
			{
				if (seqtor_at(cm->loadedChunks,i).chunkX == ceu.chunkX && seqtor_at(cm->loadedChunks, i).chunkY == ceu.chunkY && seqtor_at(cm->loadedChunks, i).chunkZ == ceu.chunkZ)
				{
					cmu.chomk = seqtor_at(cm->loadedChunks, i);
					cmu.type = CHUNKMANAGER_UNLOAD_CHUNK;
					if (ceu.isUrgent != 0)
						lista_push(cm->pendingMeshUpdates, 0, cmu);
					else
						lista_push_back(cm->pendingMeshUpdates, cmu);


					seqtor_at(cm->loadedChunks, i) = seqtor_at(cm->loadedChunks, seqtor_size(cm->loadedChunks) - 1);//a vegere rakom az eltavolitando elemet
					seqtor_remove_at(cm->loadedChunks, seqtor_size(cm->loadedChunks) - 1);

					seqtor_at(cm->isChunkCulled, i) = seqtor_at(cm->isChunkCulled, seqtor_size(cm->isChunkCulled) - 1);//a vegere rakom az eltavolitando elemet
					seqtor_remove_at(cm->isChunkCulled, seqtor_size(cm->isChunkCulled) - 1);
					break;
				}
			}
			pthread_mutex_unlock(pmutex);
			break;

	}
}

int chunkManager_updateMesh(chunkManager* cm)
{
	if (cm->pendingMeshUpdates.size == 0)
		return 0;

	lista_element_of(chunkMeshUpdate)* iterator;
	int index;

	chunkMeshUpdate cmu;

	lista_at(cm->pendingMeshUpdates, 0, &cmu);

	lista_remove_at(cm->pendingMeshUpdates, 0);

	switch (cmu.type)
	{
	case CHUNKMANAGER_LOAD_CHUNK:
		chunk_loadMeshInGPU(&cmu.chomk, cmu.meshNormal, cmu.meshWalter);
		seqtor_push_back(cm->loadedChunks, cmu.chomk);
		seqtor_push_back(cm->isChunkCulled, 0);

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
		chunk_destroy(cm, &cmu.chomk);
		break;
	}

	return 69;
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

void chunkManager_calculateFrustumCull(chunkManager* cm, mat4* pv)
{
	char isInFrustum;
	float basedX, basedY, basedZ;
	vec4 temp;

	char frustumX[3] = { 0,0,0 };//volt-e olyan bounding point, ami x<-1 vagy -1<=x<=1 vagy x>1
	char frustumY[3] = { 0,0,0 };//volt-e olyan bounding point, ami y<-1 vagy -1<=y<=1 vagy y>1
	char frustumZ[3] = { 0,0,0 };//volt-e olyan bounding point, ami z<0 vagy 0<=z<=1 vagy z>1
	char isPointInFrustum = 0;

	for (int i = 0; i < seqtor_size(cm->loadedChunks); i++)
	{
		seqtor_at(cm->isChunkCulled, i) = 69;
		if (seqtor_at(cm->loadedChunks, i).isThereNormalMesh)
		{
			basedX = seqtor_at(cm->loadedChunks, i).chunkX * CHUNK_WIDTH;
			basedY = seqtor_at(cm->loadedChunks, i).chunkY * CHUNK_HEIGHT;
			basedZ = seqtor_at(cm->loadedChunks, i).chunkZ * CHUNK_WIDTH;

			frustumX[0] = 0;	frustumX[1] = 0;	frustumX[2] = 0;
			frustumY[0] = 0;	frustumY[1] = 0;	frustumY[2] = 0;
			frustumZ[0] = 0;	frustumZ[1] = 0;	frustumZ[2] = 0;

			for (int j = 0; j < 24; )
			{
				temp.x = basedX + chunkBounds[j++];
				temp.y = basedY + chunkBounds[j++];
				temp.z = basedZ + chunkBounds[j++];
				temp.w = 1;
				temp = vec4_multiplyWithMatrix(*pv, temp);
				temp.x /= temp.w; temp.y /= temp.w; temp.z /= temp.w;//perspective division

				//frustum cull
				isPointInFrustum = chunkManager_checkIfInFrustum(&temp, frustumX, frustumY, frustumZ);

				if (isPointInFrustum)
				{
					seqtor_at(cm->isChunkCulled, i) = 0;
					break;
				}
			}

			if (!isPointInFrustum)//ha nincs pont benne a kamera frustumban
			{
				if (((frustumX[0] && frustumX[2]) || frustumX[1]) &&
					((frustumY[0] && frustumY[2]) || frustumY[1]) &&
					((frustumZ[0] && frustumZ[2]) || frustumZ[1]))
				{
					seqtor_at(cm->isChunkCulled, i) = 0;
				}
			}
		}
	}
}

int chunkManager_drawTerrain(chunkManager* cm, shader* shit, camera* cum, mat4* projection)
{
	GLuint modelLocation = glGetUniformLocation(shit->id, "model");
	mat4 pv = mat4_multiply(*projection, camera_getViewMatrix(cum));


	int drawn = 0;
	for(int i=0;i<seqtor_size(cm->loadedChunks);i++)
	{
		if (seqtor_at(cm->isChunkCulled, i) == 0 && seqtor_at(cm->loadedChunks, i).isThereNormalMesh)
		{
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, seqtor_at(cm->loadedChunks, i).model.data);
			chunk_drawTerrain(&(seqtor_at(cm->loadedChunks, i)));
			drawn++;
		}
	}
	return drawn;
}

void chunkManager_drawWalter(chunkManager* cm, shader* shit, camera* cum, mat4* projection)
{
	GLuint modelLocation = glGetUniformLocation(shit->id, "model");
	mat4 pv = mat4_multiply(*projection, camera_getViewMatrix(cum));

	for (int i = 0; i < seqtor_size(cm->loadedChunks); i++)
	{
		if (seqtor_at(cm->isChunkCulled, i) == 0 && seqtor_at(cm->loadedChunks, i).isThereNormalMesh)
		{
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, seqtor_at(cm->loadedChunks, i).model.data);
			chunk_drawWalter(&(seqtor_at(cm->loadedChunks, i)));
		}
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
	
	for(int i=0;i<seqtor_size(cm->loadedChunks);i++)
	{
		if (seqtor_at(cm->loadedChunks, i).isThereNormalMesh)
		{
			basedX = seqtor_at(cm->loadedChunks, i).chunkX * CHUNK_WIDTH;
			basedY = seqtor_at(cm->loadedChunks, i).chunkY * CHUNK_HEIGHT;
			basedZ = seqtor_at(cm->loadedChunks, i).chunkZ * CHUNK_WIDTH;

			for (int j = 0; j < 24; )
			{
				temp.x = basedX + chunkBounds[j++];
				temp.y = basedY + chunkBounds[j++];
				temp.z = basedZ + chunkBounds[j++];
				temp.w = 1;
				temp = vec4_multiplyWithMatrix(*viewProjection, temp);
				temp.x /= temp.w; temp.y /= temp.w; temp.z /= temp.w;//perspective division

				//frustum cull
				isPointInFrustum = chunkManager_checkIfInFrustum(&temp, frustumX, frustumY, frustumZ);

				if (isPointInFrustum)
				{
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, seqtor_at(cm->loadedChunks, i).model.data);
					chunk_drawTerrain(&(seqtor_at(cm->loadedChunks, i)));
					break;
				}
			}

			if (!isPointInFrustum)//ha nincs pont benne a kamera frustumban
			{
				if (((frustumX[0] && frustumX[2]) || frustumX[1]) &&
					((frustumY[0] && frustumY[2]) || frustumY[1]) &&
					((frustumZ[0] && frustumZ[2]) || frustumZ[1]))
				{
					glUniformMatrix4fv(modelLocation, 1, GL_FALSE, seqtor_at(cm->loadedChunks, i).model.data);
					chunk_drawTerrain(&(seqtor_at(cm->loadedChunks, i)));
				}
			}
		}
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

void chunkManager_reloadChunk(chunkManager* cm, pthread_mutex_t* pmutex, int chunkX, int chunkY, int chunkZ)
{
	int index = 0;
	lista_element_of(chunkGenerationUpdate)* it = NULL;
	lista_element_of(chunkMeshUpdate)* it2 = NULL;

	//remove other updates (for example a load update could cause the chunk to be loaded 2 times)
	pthread_mutex_lock(pmutex);
	for (index = 0, it = cm->pendingUpdates.head; it != NULL; it = it->next, index++)//remove every entry from pending updates
	{
		if (it->data.chunkX != chunkX)
			continue;
		if (it->data.chunkY != chunkY)
			continue;
		if (it->data.chunkZ != chunkZ)
			continue;

		it = it->next;
		lista_remove_at(cm->pendingUpdates, index);
		index++;
		if (it == NULL)
			break;
	}


	for (index = 0, it2 = cm->pendingMeshUpdates.head; it2 != NULL; it2 = it2->next, index++)//remove every entry from pending mesh updates while deleting the data that these half-generated chunks have
	{
		if (it2->data.chomk.chunkX != chunkX)
			continue;
		if (it2->data.chomk.chunkY != chunkY)
			continue;
		if (it2->data.chomk.chunkZ != chunkZ)
			continue;

		switch (it2->data.type)
		{
		case CHUNKMANAGER_LOAD_CHUNK:
			chunk_destroy(cm, &it2->data.chomk);//destroy the chunk that has been generated but not added yet

			if (it2->data.meshNormal.indexCount != 0)
			{
				free(it2->data.meshNormal.vertices);
				free(it2->data.meshNormal.indices);
			}
			if (it2->data.meshWalter.indexCount != 0)
			{
				free(it2->data.meshWalter.vertices);
				free(it2->data.meshWalter.indices);
			}
			break;

		case CHUNKMANAGER_UNLOAD_CHUNK:
			break;
		}

		it2 = it2->next;
		lista_remove_at(cm->pendingMeshUpdates, index);
		index++;
		if (it2 == NULL)
			break;
	}

	//these updates go to the front of the pendingUpdates
	
	//add a load update for the chunk
	chunkGenerationUpdate ceu2;
	ceu2.chunkX = chunkX;
	ceu2.chunkY = chunkY;
	ceu2.chunkZ = chunkZ;
	ceu2.type = CHUNKMANAGER_LOAD_CHUNK;
	ceu2.isUrgent = 69;
	lista_push(cm->pendingUpdates, 0, ceu2);
	
	//add an unload update if the chunk is already loaded
	if (chunkManager_isChunkLoaded(cm, chunkX, chunkY, chunkZ))
	{
		chunkGenerationUpdate ceu;
		ceu.chunkX = chunkX;
		ceu.chunkY = chunkY;
		ceu.chunkZ = chunkZ;
		ceu.type = CHUNKMANAGER_UNLOAD_CHUNK;
		ceu.isUrgent = 69;
		lista_push(cm->pendingUpdates, 1, ceu);
	}

	pthread_mutex_unlock(pmutex);
}

//doesnt reload the chunk
void chunkManager_changeBlock(chunkManager* cm, int chunkX, int chunkY, int chunkZ, int x, int y, int z, int type)
{
	if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_WIDTH)
		return;

	int found = 0;
	for (int i = 0; i < seqtor_size(cm->changedBlocks); i++)
	{
		if (seqtor_at(cm->changedBlocks, i).chunkX != chunkX)
			continue;
		if (seqtor_at(cm->changedBlocks, i).chunkY != chunkY)
			continue;
		if (seqtor_at(cm->changedBlocks, i).chunkZ != chunkZ)
			continue;

		found = 69;
		blockModel bm;
		bm.type = type;
		bm.x = x;
		bm.y = y;
		bm.z = z;
		seqtor_push_back(seqtor_at(cm->changedBlocks, i).blocks, bm);
		break;
	}

	if (found == 0)
	{
		blockModel bm;
		bm.type = type;
		bm.x = x;
		bm.y = y;
		bm.z = z;

		changedBlocksInChunk cbic;
		cbic.chunkX = chunkX;
		cbic.chunkY = chunkY;
		cbic.chunkZ = chunkZ;
		cbic.isRegistered = 0;
		seqtor_init(cbic.blocks, 1);
		seqtor_push_back(cbic.blocks, bm);

		seqtor_push_back(cm->changedBlocks, cbic);
	}
}

