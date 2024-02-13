#include "chunk_low_res.h"
#include "chunkManager.h"
#include "../blocks/blocks.h"
#include "../terrain/FastNoiseLite.h"

#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"

#include "../../physics/collider/collider.h"
#include "../../physics/collider_group/collider_group.h"
#include "../../physics/physics_system/physics_system.h"

#include "../../utils/seqtor.h"

#include <stdlib.h>
#include <stdio.h>
#include <glad/glad.h>
#include <string.h>
#include <math.h>

#include "chunk_ambient_occlusion.h"

static blockModel model_oak_tree[67];

static int generated = 0, destroyed = 0;


void chunkLowRes_drawTerrain(chunkLowRes* chomk)
{
	if (chomk->isThereNormalMesh == 0)
		return;

	glBindVertexArray(chomk->normalMesh.vao);
	glDrawElements(GL_TRIANGLES, chomk->normalMesh.indexCount, GL_UNSIGNED_INT, 0);
}

void chunkLowRes_drawWalter(chunkLowRes* chomk)
{
	if (chomk->isThereWaterMesh == 0)
		return;

	glBindVertexArray(chomk->waterMesh.vao);
	glDrawElements(GL_TRIANGLES, chomk->waterMesh.indexCount, GL_UNSIGNED_INT, 0);
}


chunkLowRes chunkLowRes_generate(chunkManager* cm, int chunkX, int chunkY, int chunkZ, meshRaw* meshNormal, meshRaw* meshWalter, int lodLevel)
{
	generated++;

	meshNormal->vertices = NULL; meshNormal->sizeVertices = 0;
	meshNormal->indices = NULL; meshNormal->sizeIndices = 0;
	meshNormal->indexCount = 0;

	meshWalter->vertices = NULL; meshWalter->sizeVertices = 0;
	meshWalter->indices = NULL; meshWalter->sizeIndices = 0;
	meshWalter->indexCount = 0;

	chunkLowRes chomk;
	chomk.chunkX = chunkX;
	chomk.chunkY = chunkY;
	chomk.chunkZ = chunkZ;

	int basedX = CHUNK_WIDTH * chunkX;
	int basedY = CHUNK_HEIGHT * chunkY;
	int basedZ = CHUNK_WIDTH * chunkZ;

	chomk.model = mat4_translate(mat4_create(1), vec3_create2(basedX, basedY, basedZ));

	char*** blocks = (char***)malloc((CHUNK_HEIGHT + 2) * sizeof(char**));
	for (int i = 0; i < CHUNK_HEIGHT + 2; i++)
	{
		blocks[i] = (char**)malloc((CHUNK_WIDTH + 2) * sizeof(char*));
		for (int j = 0; j < CHUNK_WIDTH + 2; j++)
			blocks[i][j] = (char*)malloc((CHUNK_WIDTH + 2) * sizeof(char));
	}



	//generating terrain
	int heightMap[CHUNK_WIDTH + 2][CHUNK_WIDTH + 2];//[x][z], a +2 az�rt van, hogy a sz�leken is ismerje a magass�got
	float heightHelper;
	for (int i = 0; i < CHUNK_WIDTH + 2; i++)
	{
		for (int j = 0; j < CHUNK_WIDTH + 2; j++)
		{
			heightHelper = fnlGetNoise2D(&(cm->noise), basedX + i, basedZ + j) * fnlGetNoise2D(&(cm->noise2), basedX + i, basedZ + j);
			heightHelper *= 200 * heightHelper;
			heightHelper += 20;
			heightHelper -= basedY;
			heightMap[i][j] = heightHelper;
		}
	}


	//filling chunk
	for (int i = 0; i < CHUNK_HEIGHT + 2; i++)//y
	{
		for (int j = 0; j < CHUNK_WIDTH + 2; j++)//x
		{
			for (int k = 0; k < CHUNK_WIDTH + 2; k++)//z 
			{
				int level = heightMap[j][k];
				if (i > level)
				{
					if (level < 28 - basedY && i < 28 - basedY)
						blocks[i][j][k] = BLOCK_WATER;
					else
						blocks[i][j][k] = BLOCK_AIR;
				}
				else if (i > level - 1)
				{
					if (level < 30 - basedY)
						blocks[i][j][k] = BLOCK_SAND;
					else
						blocks[i][j][k] = BLOCK_GRASS;
				}
				else if (i > level - 5)
				{
					if (level < 30 - basedY)
						blocks[i][j][k] = BLOCK_SAND;
					else
						blocks[i][j][k] = BLOCK_DIRT;
				}
				else
					blocks[i][j][k] = BLOCK_STONE;
			}
		}
	}


	//downscale chunk data
	int CHUNK_WIDTH_LOW_RES = CHUNK_WIDTH / lodLevel;
	int CHUNK_HEIGHT_LOW_RES = CHUNK_HEIGHT / lodLevel;
	char*** blocksLowRes = (char***)malloc((CHUNK_HEIGHT_LOW_RES + 2) * sizeof(char**));
	for (int i = 0; i < CHUNK_HEIGHT_LOW_RES + 2; i++)
	{
		blocksLowRes[i] = (char**)malloc((CHUNK_WIDTH_LOW_RES + 2) * sizeof(char*));
		for (int j = 0; j < CHUNK_WIDTH_LOW_RES + 2; j++)
			blocksLowRes[i][j] = (char*)malloc((CHUNK_WIDTH_LOW_RES + 2) * sizeof(char));
	}

	for (int i = 0; i < CHUNK_HEIGHT_LOW_RES + 2; i++)
		for (int j = 0; j < CHUNK_WIDTH_LOW_RES + 2; j++)
			for (int k = 0; k < CHUNK_WIDTH_LOW_RES + 2; k++)
				blocksLowRes[i][j][k] = BLOCK_AIR;

	//filling the inner areas
	for (int y = 0; y < CHUNK_HEIGHT_LOW_RES; y++)
	{
		for (int x = 0; x < CHUNK_WIDTH_LOW_RES; x++)
		{
			for (int z = 0; z < CHUNK_WIDTH_LOW_RES; z++)
			{
				blocksLowRes[y + 1][x + 1][z + 1] = blocks[1 + lodLevel * y][1 + lodLevel * x][1 + lodLevel * z];
			}
		}
	}

	//filling the bordering areas
	for (int y = 0; y < CHUNK_HEIGHT_LOW_RES; y++)//pos z
		for (int x = 0; x < CHUNK_HEIGHT_LOW_RES; x++)
			blocksLowRes[1 + y][1 + x][CHUNK_WIDTH_LOW_RES+1] = blocks[1 + lodLevel * y][1 + lodLevel * x][CHUNK_WIDTH+1];
	for (int y = 0; y < CHUNK_HEIGHT_LOW_RES; y++)//pos x
		for (int z = 0; z < CHUNK_WIDTH_LOW_RES; z++)
			blocksLowRes[1 + y][CHUNK_WIDTH_LOW_RES+1][1 + z] = blocks[1 + lodLevel * y][CHUNK_WIDTH+1][1 + lodLevel * z];
	for (int y = 0; y < CHUNK_HEIGHT_LOW_RES; y++)//neg z
		for (int x = 0; x < CHUNK_HEIGHT_LOW_RES; x++)
			blocksLowRes[1 + y][1 + x][0] = blocks[1 + lodLevel * y][1 + lodLevel * x][0];
	for (int y = 0; y < CHUNK_HEIGHT_LOW_RES; y++)//neg x
		for (int z = 0; z < CHUNK_WIDTH_LOW_RES; z++)
			blocksLowRes[1 + y][0][1 + z] = blocks[1 + lodLevel * y][0][1 + lodLevel * z];
	for (int x = 0; x < CHUNK_WIDTH_LOW_RES; x++)//pos y
		for (int z = 0; z < CHUNK_WIDTH_LOW_RES; z++)
			blocksLowRes[CHUNK_HEIGHT_LOW_RES+1][1 + x][1 + z] = blocks[CHUNK_HEIGHT+1][1 + lodLevel * x][1 + lodLevel * z];
	for(int x=0;x<CHUNK_WIDTH_LOW_RES;x++)//neg y
		for(int z=0;z<CHUNK_WIDTH_LOW_RES;z++)
			blocksLowRes[0][1 + x][1 + z] = blocks[0][1 + lodLevel * x][1 + lodLevel * z];



	for (int i = 0; i < CHUNK_HEIGHT + 2; i++)//deallocating the full res one
	{
		for (int j = 0; j < CHUNK_WIDTH + 2; j++)
			free(blocks[i][j]);

		free(blocks[i]);
	}
	free(blocks);
	

	//normal mesh
	seqtor_of(unsigned long) verticesNormal;
	seqtor_init(verticesNormal, 1);
	seqtor_of(unsigned int) indicesNormal;
	seqtor_init(indicesNormal, 1);

	unsigned int currentVertex = 0;//melyik az oldal elso csucsa (kell az indexeleshez)
	float x, y, z;
	unsigned long ao;
	unsigned char isBlockVisible = 0;
	for (int i = 1; i < CHUNK_HEIGHT_LOW_RES + 1; i++)//y
	{
		for (int j = 1; j < CHUNK_WIDTH_LOW_RES + 1; j++)//x
		{
			for (int k = 1; k < CHUNK_WIDTH_LOW_RES + 1; k++)//z 
			{
				if (blocksLowRes[i][j][k] == BLOCK_AIR || blocksLowRes[i][j][k] == BLOCK_WATER)
					continue;

				isBlockVisible = 0;

				//pos z
				if (blocksLowRes[i][j][k + 1] != blocksLowRes[i][j][k] && (blocksLowRes[i][j][k + 1] == BLOCK_AIR || blocksLowRes[i][j][k + 1] >= BLOCK_TRANSPARENCY_START))
				{
					//indices (6 per side (2 triangles))
					seqtor_push_back(indicesNormal, currentVertex);
					seqtor_push_back(indicesNormal, currentVertex + 2);
					seqtor_push_back(indicesNormal, currentVertex + 1);
					seqtor_push_back(indicesNormal, currentVertex + 3);
					seqtor_push_back(indicesNormal, currentVertex + 2);
					seqtor_push_back(indicesNormal, currentVertex);

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						unsigned long data = 0b0;
						chunk_ao_pos_z(blocksLowRes, j, i, k, l, ao);
						data |= (ao & 0b11u); data <<= 6;

						blocks_getVertexPosition(BLOCK_POSITIVE_Z, l, &x, &y, &z);
						data |= (j - 1 + lodLevel*lroundf(x)) & 0b111111u; data <<= 6;
						data |= (i - 1 + lodLevel*lroundf(y)) & 0b111111u; data <<= 6;
						data |= (k - 1 + lodLevel*lroundf(z)) & 0b111111u; data <<= 4;

						blocks_getUV(blocksLowRes[i][j][k], BLOCK_POSITIVE_Z, l, &x, &y);
						data |= lroundf(10 * x) & 0b1111u; data <<= 4;
						data |= lroundf(10 * y) & 0b1111u; data <<= 3;
						data |= 0b0;

						seqtor_push_back(verticesNormal, data);
					}

					currentVertex += 4;
					isBlockVisible = 1;
				}

				//pos x
				if (blocksLowRes[i][j + 1][k] != blocksLowRes[i][j][k] && (blocksLowRes[i][j + 1][k] == BLOCK_AIR || blocksLowRes[i][j + 1][k] >= BLOCK_TRANSPARENCY_START))
				{
					//indices (6 per side (2 triangles))
					seqtor_push_back(indicesNormal, currentVertex);
					seqtor_push_back(indicesNormal, currentVertex + 2);
					seqtor_push_back(indicesNormal, currentVertex + 1);
					seqtor_push_back(indicesNormal, currentVertex + 3);
					seqtor_push_back(indicesNormal, currentVertex + 2);
					seqtor_push_back(indicesNormal, currentVertex);

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						unsigned long data = 0b0;

						chunk_ao_pos_x(blocksLowRes, j, i, k, l, ao);
						data |= (ao & 0b11u); data <<= 6;

						blocks_getVertexPosition(BLOCK_POSITIVE_X, l, &x, &y, &z);
						data |= (j - 1 + lodLevel * lroundf(x)) & 0b111111u; data <<= 6;
						data |= (i - 1 + lodLevel * lroundf(y)) & 0b111111u; data <<= 6;
						data |= (k - 1 + lodLevel * lroundf(z)) & 0b111111u; data <<= 4;

						blocks_getUV(blocksLowRes[i][j][k], BLOCK_POSITIVE_X, l, &x, &y);
						data |= lroundf(10 * x) & 0b1111u; data <<= 4;
						data |= lroundf(10 * y) & 0b1111u; data <<= 3;
						data |= 0b1u;

						seqtor_push_back(verticesNormal, data);
					}
					currentVertex += 4;
					isBlockVisible = 1;
				}

				//neg z
				if (blocksLowRes[i][j][k - 1] != blocksLowRes[i][j][k] && (blocksLowRes[i][j][k - 1] == BLOCK_AIR || blocksLowRes[i][j][k - 1] >= BLOCK_TRANSPARENCY_START))
				{
					//indices (6 per side (2 triangles))
					seqtor_push_back(indicesNormal, currentVertex);
					seqtor_push_back(indicesNormal, currentVertex + 2);
					seqtor_push_back(indicesNormal, currentVertex + 1);
					seqtor_push_back(indicesNormal, currentVertex + 3);
					seqtor_push_back(indicesNormal, currentVertex + 2);
					seqtor_push_back(indicesNormal, currentVertex);

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						unsigned long data = 0b0;

						chunk_ao_neg_z(blocksLowRes, j, i, k, l, ao);
						data |= (ao & 0b11u); data <<= 6;

						blocks_getVertexPosition(BLOCK_NEGATIVE_Z, l, &x, &y, &z);
						data |= (j - 1 + lodLevel * lroundf(x)) & 0b111111u; data <<= 6;
						data |= (i - 1 + lodLevel * lroundf(y)) & 0b111111u; data <<= 6;
						data |= (k - 1 + lodLevel * lroundf(z)) & 0b111111u; data <<= 4;

						blocks_getUV(blocksLowRes[i][j][k], BLOCK_NEGATIVE_Z, l, &x, &y);
						data |= lroundf(10 * x) & 0b1111u; data <<= 4;
						data |= lroundf(10 * y) & 0b1111u; data <<= 3;
						data |= 0b10u;

						seqtor_push_back(verticesNormal, data);
					}
					currentVertex += 4;
					isBlockVisible = 1;
				}

				//neg x
				if (blocksLowRes[i][j - 1][k] != blocksLowRes[i][j][k] && (blocksLowRes[i][j - 1][k] == BLOCK_AIR || blocksLowRes[i][j - 1][k] >= BLOCK_TRANSPARENCY_START))
				{
					//indices (6 per side (2 triangles))
					seqtor_push_back(indicesNormal, currentVertex);
					seqtor_push_back(indicesNormal, currentVertex + 2);
					seqtor_push_back(indicesNormal, currentVertex + 1);
					seqtor_push_back(indicesNormal, currentVertex + 3);
					seqtor_push_back(indicesNormal, currentVertex + 2);
					seqtor_push_back(indicesNormal, currentVertex);

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						unsigned long data = 0b0;

						chunk_ao_neg_x(blocksLowRes, j, i, k, l, ao);
						data |= (ao & 0b11u); data <<= 6;

						blocks_getVertexPosition(BLOCK_NEGATIVE_X, l, &x, &y, &z);
						data |= (j - 1 + lodLevel * lroundf(x)) & 0b111111u; data <<= 6;
						data |= (i - 1 + lodLevel * lroundf(y)) & 0b111111u; data <<= 6;
						data |= (k - 1 + lodLevel * lroundf(z)) & 0b111111u; data <<= 4;

						blocks_getUV(blocksLowRes[i][j][k], BLOCK_NEGATIVE_X, l, &x, &y);
						data |= lroundf(10 * x) & 0b1111u; data <<= 4;
						data |= lroundf(10 * y) & 0b1111u; data <<= 3;
						data |= 0b11u;

						seqtor_push_back(verticesNormal, data);
					}
					currentVertex += 4;
					isBlockVisible = 1;
				}

				//pos y
				if (blocksLowRes[i + 1][j][k] != blocksLowRes[i][j][k] && (blocksLowRes[i + 1][j][k] == BLOCK_AIR || blocksLowRes[i + 1][j][k] >= BLOCK_TRANSPARENCY_START))
				{
					//indices (6 per side (2 triangles))
					seqtor_push_back(indicesNormal, currentVertex);
					seqtor_push_back(indicesNormal, currentVertex + 2);
					seqtor_push_back(indicesNormal, currentVertex + 1);
					seqtor_push_back(indicesNormal, currentVertex + 3);
					seqtor_push_back(indicesNormal, currentVertex + 2);
					seqtor_push_back(indicesNormal, currentVertex);

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						unsigned long data = 0b0;

						chunk_ao_pos_y(blocksLowRes, j, i, k, l, ao);
						data |= (ao & 0b11u); data <<= 6;

						blocks_getVertexPosition(BLOCK_POSITIVE_Y, l, &x, &y, &z);
						data |= (j - 1 + lodLevel * lroundf(x)) & 0b111111u; data <<= 6;
						data |= (i - 1 + lodLevel * lroundf(y)) & 0b111111u; data <<= 6;
						data |= (k - 1 + lodLevel * lroundf(z)) & 0b111111u; data <<= 4;

						blocks_getUV(blocksLowRes[i][j][k], BLOCK_POSITIVE_Y, l, &x, &y);
						data |= lroundf(10 * x) & 0b1111u; data <<= 4;
						data |= lroundf(10 * y) & 0b1111u; data <<= 3;
						data |= 0b100u;

						seqtor_push_back(verticesNormal, data);
					}
					currentVertex += 4;
					isBlockVisible = 1;
				}

				//neg y
				if (blocksLowRes[i - 1][j][k] != blocksLowRes[i][j][k] && (blocksLowRes[i - 1][j][k] == BLOCK_AIR || i != 0 && blocksLowRes[i - 1][j][k] >= BLOCK_TRANSPARENCY_START))
				{
					//indices (6 per side (2 triangles))
					seqtor_push_back(indicesNormal, currentVertex);
					seqtor_push_back(indicesNormal, currentVertex + 2);
					seqtor_push_back(indicesNormal, currentVertex + 1);
					seqtor_push_back(indicesNormal, currentVertex + 3);
					seqtor_push_back(indicesNormal, currentVertex + 2);
					seqtor_push_back(indicesNormal, currentVertex);

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						unsigned long data = 0b0;

						chunk_ao_neg_y(blocksLowRes, j, i, k, l, ao);
						data |= (ao & 0b11u); data <<= 6;

						blocks_getVertexPosition(BLOCK_NEGATIVE_Y, l, &x, &y, &z);
						data |= (j - 1 + lodLevel * lroundf(x)) & 0b111111u; data <<= 6;
						data |= (i - 1 + lodLevel * lroundf(y)) & 0b111111u; data <<= 6;
						data |= (k - 1 + lodLevel * lroundf(z)) & 0b111111u; data <<= 4;

						blocks_getUV(blocksLowRes[i][j][k], BLOCK_NEGATIVE_Y, l, &x, &y);
						data |= lroundf(10 * x) & 0b1111u; data <<= 4;
						data |= lroundf(10 * y) & 0b1111u; data <<= 3;
						data |= 0b101u;

						seqtor_push_back(verticesNormal, data);
					}
					currentVertex += 4;
					isBlockVisible = 1;
				}
			}
		}
	}


	//normal mesh
	if (verticesNormal.size > 0)
	{
		meshNormal->sizeVertices = verticesNormal.size * sizeof(unsigned long);
		meshNormal->vertices = malloc(meshNormal->sizeVertices);
		memcpy(meshNormal->vertices, verticesNormal.data, meshNormal->sizeVertices);

		meshNormal->sizeIndices = indicesNormal.size * sizeof(unsigned int);
		meshNormal->indices = (unsigned int*)malloc(meshNormal->sizeIndices);
		memcpy(meshNormal->indices, indicesNormal.data, meshNormal->sizeIndices);

		meshNormal->indexCount = indicesNormal.size;
	}
	seqtor_destroy(verticesNormal);
	seqtor_destroy(indicesNormal);
	chomk.isThereNormalMesh = 0;


	//walter mesh
	seqtor_of(unsigned long) verticesWalter;
	seqtor_init(verticesWalter, 1);
	seqtor_of(unsigned int) indicesWalter;
	seqtor_init(indicesWalter, 1);


	if (verticesWalter.size > 0)
	{
		meshWalter->sizeVertices = verticesWalter.size * sizeof(unsigned long);
		meshWalter->vertices = (float*)malloc(meshWalter->sizeVertices);
		memcpy(meshWalter->vertices, verticesWalter.data, meshWalter->sizeVertices);

		meshWalter->sizeIndices = indicesWalter.size * sizeof(unsigned int);
		meshWalter->indices = (unsigned int*)malloc(meshWalter->sizeIndices);
		memcpy(meshWalter->indices, indicesWalter.data, meshWalter->sizeIndices);

		meshWalter->indexCount = indicesWalter.size;
	}

	seqtor_destroy(verticesWalter);
	seqtor_destroy(indicesWalter);
	chomk.isThereWaterMesh = 0;

	//destroy the blocks
	for (int i = 0; i < CHUNK_HEIGHT_LOW_RES + 2; i++)
	{
		for (int j = 0; j < CHUNK_WIDTH_LOW_RES + 2; j++)
			free(blocksLowRes[i][j]);

		free(blocksLowRes[i]);
	}
	free(blocksLowRes);

	return chomk;
}

void chunkLowRes_loadMeshInGPU(chunkLowRes* chomk, meshRaw meshNormal, meshRaw meshWalter)
{
	if (meshNormal.indexCount == 0)
	{
		chomk->isThereNormalMesh = 0;
		chomk->normalMesh.indexCount = 0;
	}
	else
	{
		glGenVertexArrays(1, &chomk->normalMesh.vao);
		glBindVertexArray(chomk->normalMesh.vao);

		glGenBuffers(1, &chomk->normalMesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, chomk->normalMesh.vbo);
		glBufferData(GL_ARRAY_BUFFER, meshNormal.sizeVertices, meshNormal.vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &chomk->normalMesh.ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chomk->normalMesh.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshNormal.sizeIndices, meshNormal.indices, GL_STATIC_DRAW);

		/*glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);//positions
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);//uv and geom
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);//normal
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(3);//tangent*/
		glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(unsigned long), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);

		chomk->normalMesh.indexCount = meshNormal.indexCount;
		chomk->isThereNormalMesh = 69;
	}


	if (meshWalter.indexCount == 0)
	{
		chomk->isThereWaterMesh = 0;
		chomk->waterMesh.indexCount = 0;
	}
	else
	{
		glGenVertexArrays(1, &chomk->waterMesh.vao);
		glBindVertexArray(chomk->waterMesh.vao);

		glGenBuffers(1, &chomk->waterMesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, chomk->waterMesh.vbo);
		glBufferData(GL_ARRAY_BUFFER, meshWalter.sizeVertices, meshWalter.vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &chomk->waterMesh.ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chomk->waterMesh.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshWalter.sizeIndices, meshWalter.indices, GL_STATIC_DRAW);

		glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(unsigned long) + 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(unsigned long) + 2 * sizeof(float), (void*)(sizeof(unsigned long)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);

		chomk->waterMesh.indexCount = meshWalter.indexCount;
		chomk->isThereWaterMesh = 69;
	}
}

void chunkLowRes_destroy(chunkManager* cm, chunkLowRes* chomk)
{
	destroyed++;

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

void chunkLowRes_resetGenerationInfo()
{
	generated = 0;
	destroyed = 0;
}

void chunkLowRes_getGenerationInfo(int* _generated, int* _destroyed)
{
	*_generated = generated;
	*_destroyed = destroyed;
}

void chunkLowRes_getChunkFromPos(vec3 pos, int* chunkX, int* chunkY, int* chunkZ)
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

static blockModel model_oak_tree[] = {
	{0,0,0,BLOCK_DIRT},
	{0,1,0,BLOCK_OAK_LOG},
	{0,2,0,BLOCK_OAK_LOG},
	{0,3,0,BLOCK_OAK_LOG},
	{0,4,0,BLOCK_OAK_LOG},
	{0,5,0,BLOCK_OAK_LOG},
	{-2,3,-2, BLOCK_OAK_LEAVES},
	{-2,3,-1, BLOCK_OAK_LEAVES},
	{-2,3,0, BLOCK_OAK_LEAVES},
	{-2,3,1, BLOCK_OAK_LEAVES},
	{-2,3,2, BLOCK_OAK_LEAVES},
	{-1,3,-2, BLOCK_OAK_LEAVES},
	{-1,3,-1, BLOCK_OAK_LEAVES},
	{-1,3,0, BLOCK_OAK_LEAVES},
	{-1,3,1, BLOCK_OAK_LEAVES},
	{-1,3,2, BLOCK_OAK_LEAVES},
	{0,3,-2, BLOCK_OAK_LEAVES},
	{0,3,-1, BLOCK_OAK_LEAVES},
	{0,3,1, BLOCK_OAK_LEAVES},
	{0,3,2, BLOCK_OAK_LEAVES},
	{1,3,-2, BLOCK_OAK_LEAVES},
	{1,3,-1, BLOCK_OAK_LEAVES},
	{1,3,0, BLOCK_OAK_LEAVES},
	{1,3,1, BLOCK_OAK_LEAVES},
	{1,3,2, BLOCK_OAK_LEAVES},
	{2,3,-2, BLOCK_OAK_LEAVES},
	{2,3,-1, BLOCK_OAK_LEAVES},
	{2,3,0, BLOCK_OAK_LEAVES},
	{2,3,1, BLOCK_OAK_LEAVES},
	{2,3,2, BLOCK_OAK_LEAVES},
	{-2,4,-2, BLOCK_OAK_LEAVES},
	{-2,4,-1, BLOCK_OAK_LEAVES},
	{-2,4,0, BLOCK_OAK_LEAVES},
	{-2,4,1, BLOCK_OAK_LEAVES},
	{-2,4,2, BLOCK_OAK_LEAVES},
	{-1,4,-2, BLOCK_OAK_LEAVES},
	{-1,4,-1, BLOCK_OAK_LEAVES},
	{-1,4,0, BLOCK_OAK_LEAVES},
	{-1,4,1, BLOCK_OAK_LEAVES},
	{-1,4,2, BLOCK_OAK_LEAVES},
	{0,4,-2, BLOCK_OAK_LEAVES},
	{0,4,-1, BLOCK_OAK_LEAVES},
	{0,4,1, BLOCK_OAK_LEAVES},
	{0,4,2, BLOCK_OAK_LEAVES},
	{1,4,-2, BLOCK_OAK_LEAVES},
	{1,4,-1, BLOCK_OAK_LEAVES},
	{1,4,0, BLOCK_OAK_LEAVES},
	{1,4,1, BLOCK_OAK_LEAVES},
	{1,4,2, BLOCK_OAK_LEAVES},
	{2,4,-2, BLOCK_OAK_LEAVES},
	{2,4,-1, BLOCK_OAK_LEAVES},
	{2,4,0, BLOCK_OAK_LEAVES},
	{2,4,1, BLOCK_OAK_LEAVES},
	{2,4,2, BLOCK_OAK_LEAVES},
	{-1,5,-1, BLOCK_OAK_LEAVES},
	{-1,5,-0, BLOCK_OAK_LEAVES},
	{-1,5,1, BLOCK_OAK_LEAVES},
	{0,5,-1, BLOCK_OAK_LEAVES},
	{0,5,1, BLOCK_OAK_LEAVES},
	{1,5,-1, BLOCK_OAK_LEAVES},
	{1,5,0, BLOCK_OAK_LEAVES},
	{1,5,1, BLOCK_OAK_LEAVES},
	{0,6,-1, BLOCK_OAK_LEAVES},
	{0,6,1, BLOCK_OAK_LEAVES},
	{-1,6,0, BLOCK_OAK_LEAVES},
	{1,6,0, BLOCK_OAK_LEAVES},
	{0,6,0, BLOCK_OAK_LEAVES}
};