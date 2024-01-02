#include "chunk.h"
#include "chunkManager.h"
#include "../blocks/blocks.h"
#include <stdlib.h>
#include <glad/glad.h>
#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"

void chunk_drawTerrain(chunk* chomk)
{
	if (chomk->isThereNormalMesh == 0)
		return;

	glBindVertexArray(chomk->normalMesh.vao);
	glDrawElements(GL_TRIANGLES, chomk->normalMesh.indexCount, GL_UNSIGNED_INT, 0);
}

void chunk_drawWalter(chunk* chomk)
{
	if (chomk->isThereWaterMesh == 0)
		return;

	glBindVertexArray(chomk->waterMesh.vao);
	glDrawElements(GL_TRIANGLES, chomk->waterMesh.indexCount, GL_UNSIGNED_INT, 0);
}


chunk chunk_generate(chunkManager* cm, int chunkX, int chunkY, int chunkZ)
{
	chunk chomk;
	chomk.chunkX = chunkX;
	chomk.chunkY = chunkY;
	chomk.chunkZ = chunkZ;
 
	int basedX = CHUNK_WIDTH * chunkX;
	int basedY = CHUNK_HEIGHT * chunkY;
	int basedZ = CHUNK_WIDTH * chunkZ;

	chomk.model = mat4_translate(mat4_create(1), vec3_create2(basedX, basedY, basedZ));

	chomk.blocks = (char***)malloc(CHUNK_HEIGHT * sizeof(char**));
	for (int i = 0; i < CHUNK_WIDTH; i++)
	{
		chomk.blocks[i] = (char**)malloc(CHUNK_WIDTH * sizeof(char*));
		for (int j = 0; j < CHUNK_WIDTH; j++)
			chomk.blocks[i][j] = (char*)malloc(CHUNK_WIDTH * sizeof(char));
	}

	int heightMap[CHUNK_WIDTH + 2][CHUNK_WIDTH + 2];//[x][z], a +2 az�rt van, hogy a sz�leken is ismerje a magass�got
	float heightHelper;
	for (int i = 0; i < CHUNK_WIDTH + 2; i++)
	{
		for (int j = 0; j < CHUNK_WIDTH + 2; j++)
		{
			heightHelper = fnlGetNoise2D(&(cm->noise), basedX + i, basedZ + j);
			heightHelper *= 1000*heightHelper;
			heightHelper -= basedY - 20;
			heightMap[i][j] = heightHelper;
		}
	}
	

	//filling chunk
	for (int i = 0; i < CHUNK_HEIGHT; i++)//y
	{
		for (int j = 0; j < CHUNK_WIDTH; j++)//x
		{
			for (int k = 0; k < CHUNK_WIDTH; k++)//z 
			{
				int level = heightMap[j + 1][k + 1];
				if (i >  level)
					chomk.blocks[i][j][k] = BLOCK_AIR;
				else if(i>level-1)
					chomk.blocks[i][j][k] = BLOCK_GRASS;
				else if (i > level - 8)
					chomk.blocks[i][j][k] = BLOCK_DIRT;
				else
					chomk.blocks[i][j][k] = BLOCK_STONE;
			}
		}
	}

	//generating mesh
	//egyelore csak chunkon belul keres megjelenitendo oldalakat
	int sideCount = 0;
	for (int i = 0; i < CHUNK_HEIGHT; i++)//y
	{
		for (int j = 0; j < CHUNK_WIDTH; j++)//x
		{
			for (int k = 0; k < CHUNK_WIDTH; k++)//z 
			{
				if (chomk.blocks[i][j][k] == BLOCK_AIR)
					continue;

				//pos z
				if ((k == CHUNK_WIDTH - 1 && heightMap[j+1][CHUNK_WIDTH + 1]<i) || (k != CHUNK_WIDTH - 1 &&chomk.blocks[i][j][k + 1] == BLOCK_AIR))
					sideCount++;

				//pos x
				if ((j == CHUNK_WIDTH - 1 && heightMap[CHUNK_WIDTH + 1][k+1]<i) || (j!=CHUNK_WIDTH-1 &&chomk.blocks[i][j+1][k] == BLOCK_AIR))
					sideCount++;

				//neg z
				if ((k == 0 && heightMap[j+1][0]<i) || (k!=0&&chomk.blocks[i][j][k -1] == BLOCK_AIR))
					sideCount++;

				//neg x
				if ((j == 0 &&heightMap[0][k+1]<i) || (j!=0&&chomk.blocks[i][j - 1][k] == BLOCK_AIR))
					sideCount++;

				//pos y
				if ((i == CHUNK_HEIGHT - 1 && heightMap[j+1][k+1] ==CHUNK_HEIGHT-1) || (i!=CHUNK_HEIGHT-1&&chomk.blocks[i + 1][j][k] == BLOCK_AIR))
					sideCount++;

				//neg y
				if ((i == 0 &&basedY==0) || (i!=0&&chomk.blocks[i - 1][j][k] == BLOCK_AIR))
					sideCount++;
			}
		}
	}

	if (sideCount == 0)
	{
		chomk.isThereNormalMesh = 0;
		goto WalterGeneration;
	}

	float* normalVertexData = (float*)malloc(4 * sideCount * CHUNK_NORMAL_MESH_VERTEX_SIZE* sizeof(float));
	unsigned int* normalVertexIndices=(unsigned int*)malloc(6 * sideCount * sizeof(unsigned int));

	unsigned int currentVertex = 0;//melyik az oldal elso csucsa (kell az indexeleshez)
	unsigned int vertexDataIndex = 0;//eppen hol tartunk a normalVertexData tombon belul
	unsigned int vertexIndexIndex = 0;
	float x, y, z;
	for (int i = 0; i < CHUNK_HEIGHT; i++)//y
	{
		for (int j = 0; j < CHUNK_WIDTH; j++)//x
		{
			for (int k = 0; k < CHUNK_WIDTH; k++)//z 
			{
				if (chomk.blocks[i][j][k] == BLOCK_AIR)
					continue;

				//pos z
				if ((k == CHUNK_WIDTH - 1 && heightMap[j + 1][CHUNK_WIDTH + 1] < i) || (k != CHUNK_WIDTH - 1 && chomk.blocks[i][j][k + 1] == BLOCK_AIR))
				{
					//indices (6 per side (2 triangles))
					normalVertexIndices[vertexIndexIndex++] = currentVertex;
					normalVertexIndices[vertexIndexIndex++] = currentVertex+2;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 1;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 3;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 2;
					normalVertexIndices[vertexIndexIndex++] = currentVertex;

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						blocks_getVertexPosition(BLOCK_POSITIVE_Z, l, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = j+x;
						normalVertexData[vertexDataIndex++] = i+y;
						normalVertexData[vertexDataIndex++] = k+z;

						blocks_getUV(chomk.blocks[i][j][k],BLOCK_POSITIVE_Z, l, &x, &y);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = 0;//geometry

						blocks_getVertexNormal(BLOCK_POSITIVE_Z, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;

						blocks_getVertexTangent(BLOCK_POSITIVE_Z, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;

						blocks_getVertexBitangent(BLOCK_POSITIVE_Z, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;
					}

					currentVertex += 4;
				}

				//pos x
				if ((j == CHUNK_WIDTH - 1 && heightMap[CHUNK_WIDTH + 1][k + 1] < i) || (j != CHUNK_WIDTH - 1 && chomk.blocks[i][j + 1][k] == BLOCK_AIR))
				{
					//indices (6 per side (2 triangles))
					normalVertexIndices[vertexIndexIndex++] = currentVertex;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 2;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 1;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 3;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 2;
					normalVertexIndices[vertexIndexIndex++] = currentVertex;

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						blocks_getVertexPosition(BLOCK_POSITIVE_X, l, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = j + x;
						normalVertexData[vertexDataIndex++] = i + y;
						normalVertexData[vertexDataIndex++] = k + z;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_POSITIVE_X, l, &x, &y);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = 0;//geometry

						blocks_getVertexNormal(BLOCK_POSITIVE_X, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;

						blocks_getVertexTangent(BLOCK_POSITIVE_X, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;

						blocks_getVertexBitangent(BLOCK_POSITIVE_X, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;
					}
					currentVertex += 4;
				}

				//neg z
				if ((k == 0 && heightMap[j + 1][0] < i) || (k != 0 && chomk.blocks[i][j][k - 1] == BLOCK_AIR))
				{
					//indices (6 per side (2 triangles))
					normalVertexIndices[vertexIndexIndex++] = currentVertex;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 2;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 1;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 3;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 2;
					normalVertexIndices[vertexIndexIndex++] = currentVertex;

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						blocks_getVertexPosition(BLOCK_NEGATIVE_Z, l, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = j + x;
						normalVertexData[vertexDataIndex++] = i + y;
						normalVertexData[vertexDataIndex++] = k + z;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_NEGATIVE_Z, l, &x, &y);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = 0;//geometry

						blocks_getVertexNormal(BLOCK_NEGATIVE_Z, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;

						blocks_getVertexTangent(BLOCK_NEGATIVE_Z, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;

						blocks_getVertexBitangent(BLOCK_NEGATIVE_Z, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;
					}
					currentVertex += 4;
				}

				//neg x
				if ((j == 0 && heightMap[0][k + 1] < i) || (j != 0 && chomk.blocks[i][j - 1][k] == BLOCK_AIR))
				{
					//indices (6 per side (2 triangles))
					normalVertexIndices[vertexIndexIndex++] = currentVertex;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 2;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 1;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 3;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 2;
					normalVertexIndices[vertexIndexIndex++] = currentVertex;

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						blocks_getVertexPosition(BLOCK_NEGATIVE_X, l, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = j + x;
						normalVertexData[vertexDataIndex++] = i + y;
						normalVertexData[vertexDataIndex++] = k + z;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_NEGATIVE_X, l, &x, &y);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = 0;//geometry

						blocks_getVertexNormal(BLOCK_NEGATIVE_X, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;

						blocks_getVertexTangent(BLOCK_NEGATIVE_X, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;

						blocks_getVertexBitangent(BLOCK_NEGATIVE_X, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;
					}
					currentVertex += 4;
				}

				//pos y
				if ((i == CHUNK_HEIGHT - 1 && heightMap[j + 1][k + 1] == CHUNK_HEIGHT - 1) || (i != CHUNK_HEIGHT - 1 && chomk.blocks[i + 1][j][k] == BLOCK_AIR))
				{
					//indices (6 per side (2 triangles))
					normalVertexIndices[vertexIndexIndex++] = currentVertex;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 2;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 1;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 3;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 2;
					normalVertexIndices[vertexIndexIndex++] = currentVertex;

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						blocks_getVertexPosition(BLOCK_POSITIVE_Y, l, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = j + x;
						normalVertexData[vertexDataIndex++] = i + y;
						normalVertexData[vertexDataIndex++] = k + z;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_POSITIVE_Y, l, &x, &y);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = 0;//geometry

						blocks_getVertexNormal(BLOCK_POSITIVE_Y, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;

						blocks_getVertexTangent(BLOCK_POSITIVE_Y, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;

						blocks_getVertexBitangent(BLOCK_POSITIVE_Y, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;
					}
					currentVertex += 4;
				}

				//neg y
				if ((i == 0 && basedY == 0) || (i != 0 && chomk.blocks[i - 1][j][k] == BLOCK_AIR))
				{
					//indices (6 per side (2 triangles))
					normalVertexIndices[vertexIndexIndex++] = currentVertex;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 2;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 1;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 3;
					normalVertexIndices[vertexIndexIndex++] = currentVertex + 2;
					normalVertexIndices[vertexIndexIndex++] = currentVertex;

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						blocks_getVertexPosition(BLOCK_NEGATIVE_Y, l, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = j + x;
						normalVertexData[vertexDataIndex++] = i + y;
						normalVertexData[vertexDataIndex++] = k + z;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_NEGATIVE_Y, l, &x, &y);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = 0;//geometry

						blocks_getVertexNormal(BLOCK_NEGATIVE_Y, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;

						blocks_getVertexTangent(BLOCK_NEGATIVE_Y, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;

						blocks_getVertexBitangent(BLOCK_NEGATIVE_Y, &x, &y, &z);
						normalVertexData[vertexDataIndex++] = x;
						normalVertexData[vertexDataIndex++] = y;
						normalVertexData[vertexDataIndex++] = z;
					}
					currentVertex += 4;
				}
			}
		}
	}

	chomk.normalMesh.indexCount = 6 * sideCount;

	glGenVertexArrays(1, &chomk.normalMesh.vao);
	glBindVertexArray(chomk.normalMesh.vao);

	glGenBuffers(1, &chomk.normalMesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, chomk.normalMesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sideCount * CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), normalVertexData, GL_STATIC_DRAW);

	glGenBuffers(1, &chomk.normalMesh.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chomk.normalMesh.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sideCount * sizeof(unsigned int), normalVertexIndices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);//positions
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);//uv and geom
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);//normal
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);//tangent
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(12 * sizeof(float)));
	glEnableVertexAttribArray(4);//bitangent

	glBindVertexArray(0);

	chomk.isThereNormalMesh = 69;
	
	free(normalVertexData);
	free(normalVertexIndices);

WalterGeneration:

	chomk.isThereWaterMesh = 0;

	return chomk;
}

void chunk_destroy(chunk* chomk)
{
	for (int i = 0; i < CHUNK_WIDTH; i++)
	{
		for (int j = 0; j < CHUNK_WIDTH; j++)
			free(chomk->blocks[i][j]);

		free(chomk->blocks[i]);
	}
	free(chomk->blocks);

	if (chomk->isThereNormalMesh != 0) {
		chomk->isThereNormalMesh = 0;
		glDeleteVertexArrays(1, &(chomk->normalMesh.vao));
		glDeleteBuffers(1, &(chomk->normalMesh.vbo));
		glDeleteBuffers(1, &(chomk->normalMesh.ebo));
	}
	if (chomk->isThereWaterMesh != 0) {
		chomk->isThereWaterMesh = 0;
		glDeleteVertexArrays(1, &(chomk->waterMesh.vao));
		glDeleteBuffers(1, &(chomk->waterMesh.vbo));
		glDeleteBuffers(1, &(chomk->waterMesh.ebo));
	}
}

void chunk_getChunkFromPos(vec3 pos, int* chunkX, int* chunkY, int* chunkZ)
{
	if (pos.x < 0) {
		*chunkX = ((int)(pos.x - CHUNK_WIDTH)) / CHUNK_WIDTH;
	}
	else {
		*chunkX = ((int)(pos.x)) / CHUNK_WIDTH;
	}

	if (pos.y < 0) {
		*chunkY = ((int)(pos.y - CHUNK_HEIGHT)) / CHUNK_HEIGHT;
	}
	else {
		*chunkY = ((int)(pos.y)) / CHUNK_HEIGHT;
	}

	if (pos.z < 0) {
		*chunkZ = ((int)(pos.z - CHUNK_WIDTH)) / CHUNK_WIDTH;
	}
	else {
		*chunkZ = ((int)(pos.z)) / CHUNK_WIDTH;
	}
}