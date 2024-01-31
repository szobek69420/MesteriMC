#include "chunk.h"
#include "chunkManager.h"
#include "../blocks/blocks.h"
#include "../terrain/FastNoiseLite.h"

#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"
#include "../../utils/seqtor.h"

#include <stdlib.h>
#include <stdio.h>
#include <glad/glad.h>
#include <string.h>
#include <math.h>

static blockModel model_oak_tree[67];


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


chunk chunk_generate(chunkManager* cm, int chunkX, int chunkY, int chunkZ, meshRaw* meshNormal, meshRaw* meshWalter, int firstLoad)
{
	meshNormal->vertices = NULL; meshNormal->sizeVertices = 0;
	meshNormal->indices = NULL; meshNormal->sizeIndices = 0;
	meshNormal->indexCount = 0;

	meshWalter->vertices = NULL; meshWalter->sizeVertices = 0;
	meshWalter->indices = NULL; meshWalter->sizeIndices = 0;
	meshWalter->indexCount = 0;

	chunk chomk;
	chomk.chunkX = chunkX;
	chomk.chunkY = chunkY;
	chomk.chunkZ = chunkZ;
 
	int basedX = CHUNK_WIDTH * chunkX;
	int basedY = CHUNK_HEIGHT * chunkY;
	int basedZ = CHUNK_WIDTH * chunkZ;

	chomk.model = mat4_translate(mat4_create(1), vec3_create2(basedX, basedY, basedZ));

	chomk.blocks = (char***)malloc((CHUNK_HEIGHT+2) * sizeof(char**));
	for (int i = 0; i < CHUNK_WIDTH+2; i++)
	{
		chomk.blocks[i] = (char**)malloc((CHUNK_WIDTH+2) * sizeof(char*));
		for (int j = 0; j < CHUNK_WIDTH+2; j++)
			chomk.blocks[i][j] = (char*)malloc((CHUNK_WIDTH+2) * sizeof(char));
	}

	//getting neighbouring chunk data
	seqtor_of(blockModel)* current=NULL, *plusZ=NULL, *plusX = NULL, *minusZ = NULL, *minusX = NULL, *plusY = NULL, *minusY = NULL;


	//ha a firstLoad!=0, akkor nem letezik a chunkhoz tartozo bejegyzes a changedBlocksban.
	//ekkor letrehoz egy vektort a currenthez, amelyhez hozzaadja a hozzaadando blokkokat a chunk generalasa kozben
	//amennyiben a generalas vegen a vektor nem ures, azt hozzaadja a changedBlocks-hoz
	if (firstLoad)
	{
		current = malloc(sizeof(seqtor_of(blockModel)));
		seqtor_init(*current, 1);
	}
	else
	{
		for (int i = 0; i < seqtor_size(cm->changedBlocks); i++)
		{
			if (seqtor_at(cm->changedBlocks, i).chunkX != chunkX)
				continue;
			if (seqtor_at(cm->changedBlocks, i).chunkY != chunkY)
				continue;
			if (seqtor_at(cm->changedBlocks, i).chunkZ != chunkZ)
				continue;

			current = &seqtor_at(cm->changedBlocks, i).blocks;
			break;
		}
	}

	int tempX, tempY, tempZ, zeroCount;
	for (int i = 0; i < cm->changedBlocks.size; i++)
	{
		zeroCount = 0;
		tempX = seqtor_at(cm->changedBlocks, i).chunkX - chunkX;
		tempY = seqtor_at(cm->changedBlocks, i).chunkY - chunkY;
		tempZ = seqtor_at(cm->changedBlocks, i).chunkZ - chunkZ;

		if (tempX < -1 || tempX>1)
			continue;
		if (tempY < -1 || tempY>1)
			continue;
		if (tempZ < -1 || tempY>1)
			continue;

		if (tempX == 0)
			zeroCount++;
		if (tempY == 0)
			zeroCount++;
		if (tempZ == 0)
			zeroCount++;

		if (zeroCount != 2)
			continue;

		if (tempX == 1)
			plusX = &seqtor_at(cm->changedBlocks, i).blocks;
		if (tempX == -1)
			minusX = &seqtor_at(cm->changedBlocks, i).blocks;
		if (tempY == 1)
			plusY = &seqtor_at(cm->changedBlocks, i).blocks;
		if (tempY == -1)
			minusY = &seqtor_at(cm->changedBlocks, i).blocks;
		if (tempZ == 1)
			plusZ = &seqtor_at(cm->changedBlocks, i).blocks;
		if (tempZ == -1)
			minusZ = &seqtor_at(cm->changedBlocks, i).blocks;
	}


	//generating terrain
	int heightMap[CHUNK_WIDTH + 2][CHUNK_WIDTH + 2];//[x][z], a +2 az�rt van, hogy a sz�leken is ismerje a magass�got
	float heightHelper;
	for (int i = 0; i < CHUNK_WIDTH + 2; i++)
	{
		for (int j = 0; j < CHUNK_WIDTH + 2; j++)
		{
			heightHelper = fnlGetNoise2D(&(cm->noise), basedX + i, basedZ + j)*fnlGetNoise2D(&(cm->noise2),basedX+i, basedZ+j);
			heightHelper *= 200*heightHelper;
			heightHelper += 20;
			heightHelper -= basedY;
			heightMap[i][j] = heightHelper;
		}
	}
	

	//filling chunk
	for (int i = 0; i < CHUNK_HEIGHT+2; i++)//y
	{
		for (int j = 0; j < CHUNK_WIDTH+2; j++)//x
		{
			for (int k = 0; k < CHUNK_WIDTH+2; k++)//z 
			{
				int level = heightMap[j][k];
				if (i > level)
				{
					if(level<28-basedY&&i<28-basedY)
						chomk.blocks[i][j][k] = BLOCK_WATER;
					else
						chomk.blocks[i][j][k] = BLOCK_AIR;
				}
				else if (i > level - 1)
				{
					if(level<30-basedY)
						chomk.blocks[i][j][k] = BLOCK_SAND;
					else
						chomk.blocks[i][j][k] = BLOCK_GRASS;
				}
				else if (i > level - 5)
				{
					if (level < 30-basedY)
						chomk.blocks[i][j][k] = BLOCK_SAND;
					else
						chomk.blocks[i][j][k] = BLOCK_DIRT;
				}
				else
					chomk.blocks[i][j][k] = BLOCK_STONE;
			}
		}
	}

	//things that should be generated only once and then stored in the changedBlocks (for example trees)
	if (firstLoad)
	{
		//adding trees
		srand(1000000 * chunkX + 1000 * chunkY + chunkZ);
		for (int i = 4; i < CHUNK_WIDTH - 1; i++)
		{
			for (int j = 4; j < CHUNK_WIDTH - 1; j++)
			{
				int height = heightMap[i + 1][j + 1];
				if (height >= 1 && height < CHUNK_HEIGHT - 5)
				{
					float szam = (float)rand() / RAND_MAX;
					if (chomk.blocks[height][i][j] == BLOCK_GRASS && szam > 0.995f)
					{
						for (int k = 0; k < sizeof(model_oak_tree) / sizeof(blockModel); k++)
						{
							blockModel bm = model_oak_tree[k];
							if (chomk.blocks[height + bm.y][i + bm.x][j + bm.z] == BLOCK_AIR)
							{
								//a blokkok indexe a vektorban chunkhoz relativ, azaz 0 es CHUNK_WIDTH-1 kozott van
								//ehhez hozza kell adni egyet, hogy megkapjuk a chomk.blocks-beli indexet
								chomk.blocks[height + bm.y][i + bm.x][j + bm.z] = bm.type;
								blockModel pushed = (blockModel){ i-1 + bm.x, height-1 + bm.y, j-1 + bm.z };
								seqtor_push_back(*current, pushed);
							}
						}
					}
				}
			}
		}
	}

	//adding the contents of the current and neightbouring chunks
	for (int i = 0; i < current->size; i++)
		chomk.blocks[seqtor_at(*current, i).y+1][seqtor_at(*current, i).x+1][seqtor_at(*current, i).z+1] = seqtor_at(*current, i).type;

	if (plusZ != NULL)
	{
		for (int i = 0; i < plusZ->size; i++)
		{
			if(seqtor_at(*plusZ, i).z==0)
				chomk.blocks[seqtor_at(*plusZ, i).y+1][seqtor_at(*plusZ, i).x+1][CHUNK_WIDTH+1] = seqtor_at(*plusZ, i).type;
		}
	}
	if (plusX != NULL)
	{
		for (int i = 0; i < plusX->size; i++)
		{
			if (seqtor_at(*plusX, i).x == 0)
				chomk.blocks[seqtor_at(*plusX, i).y+1][CHUNK_WIDTH+1][seqtor_at(*plusX, i).z+1] = seqtor_at(*plusX, i).type;
		}
	}
	if (minusZ != NULL)
	{
		for (int i = 0; i < minusZ->size; i++)
		{
			if(seqtor_at(*minusZ,i).z==CHUNK_WIDTH-1)
				chomk.blocks[seqtor_at(*minusZ, i).y+1][seqtor_at(*minusZ, i).x+1][0] = seqtor_at(*minusZ, i).type;
		}
	}
	if (minusX != NULL)
	{
		for (int i = 0; i < minusX->size; i++)
		{
			if (seqtor_at(*minusX, i).x == CHUNK_WIDTH - 1)
				chomk.blocks[seqtor_at(*minusX, i).y+1][0][seqtor_at(*minusX, i).z+1] = seqtor_at(*minusX, i).type;
		}
	}
	if (plusY != NULL)
	{
		for (int i = 0; i < plusY->size; i++)
		{
			if (seqtor_at(*plusY, i).y == 0)
				chomk.blocks[CHUNK_HEIGHT+1][seqtor_at(*plusY, i).z + 1][seqtor_at(*plusY, i).z + 1] = seqtor_at(*plusY, i).type;
		}
	}
	if (minusY != NULL)
	{
		for (int i = 0; i < minusY->size; i++)
		{
			if (seqtor_at(*minusY, i).y == CHUNK_HEIGHT-1)
				chomk.blocks[0][seqtor_at(*minusY, i).z + 1][seqtor_at(*minusY, i).z + 1] = seqtor_at(*minusY, i).type;
		}
	}

	//normal mesh
	seqtor_of(unsigned long) verticesNormal;
	seqtor_init(verticesNormal, 1);
	seqtor_of(unsigned int) indicesNormal;
	seqtor_init(indicesNormal, 1);

	unsigned int currentVertex = 0;//melyik az oldal elso csucsa (kell az indexeleshez)
	float x, y, z;
	for (int i = 1; i < CHUNK_HEIGHT+1; i++)//y
	{
		for (int j = 1; j < CHUNK_WIDTH+1; j++)//x
		{
			for (int k = 1; k < CHUNK_WIDTH+1; k++)//z 
			{
				if (chomk.blocks[i][j][k] == BLOCK_AIR||chomk.blocks[i][j][k]==BLOCK_WATER)
					continue;

				//pos z
				if (chomk.blocks[i][j][k + 1] != chomk.blocks[i][j][k] && (chomk.blocks[i][j][k + 1] == BLOCK_AIR || chomk.blocks[i][j][k + 1] >= BLOCK_TRANSPARENCY_START))
				{
					//indices (6 per side (2 triangles))
					seqtor_push_back(indicesNormal, currentVertex);
					seqtor_push_back(indicesNormal, currentVertex+2);
					seqtor_push_back(indicesNormal, currentVertex+1);
					seqtor_push_back(indicesNormal, currentVertex+3);
					seqtor_push_back(indicesNormal, currentVertex+2);
					seqtor_push_back(indicesNormal, currentVertex);

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						unsigned long data = 0b0;
						blocks_getVertexPosition(BLOCK_POSITIVE_Z, l, &x, &y, &z);
						data |= (j-1 + lroundf(x)) & 0b111111u; data <<= 6;
						data |= (i-1 + lroundf(y)) & 0b111111u; data <<= 6;
						data |= (k-1 + lroundf(z)) & 0b111111u; data <<= 4;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_POSITIVE_Z, l, &x, &y);
						data |= lroundf(10 * x) & 0b1111u; data <<= 4;
						data |= lroundf(10 * y) & 0b1111u; data <<= 3;
						data |= 0b0;

						seqtor_push_back(verticesNormal, data);
					}

					currentVertex += 4;
				}

				//pos x
				if (chomk.blocks[i][j + 1][k] != chomk.blocks[i][j][k] && (chomk.blocks[i][j + 1][k] == BLOCK_AIR || chomk.blocks[i][j + 1][k] >= BLOCK_TRANSPARENCY_START))
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
						blocks_getVertexPosition(BLOCK_POSITIVE_X, l, &x, &y, &z);
						data |= (j-1 + lroundf(x)) & 0b111111u; data <<= 6;
						data |= (i-1 + lroundf(y)) & 0b111111u; data <<= 6;
						data |= (k-1 + lroundf(z)) & 0b111111u; data <<= 4;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_POSITIVE_X, l, &x, &y);
						data |= lroundf(10 * x) & 0b1111u; data <<= 4;
						data |= lroundf(10 * y) & 0b1111u; data <<= 3;
						data |= 0b1u;

						seqtor_push_back(verticesNormal, data);
					}
					currentVertex += 4;
				}

				//neg z
				if (chomk.blocks[i][j][k - 1] != chomk.blocks[i][j][k] && (chomk.blocks[i][j][k - 1] == BLOCK_AIR || chomk.blocks[i][j][k - 1] >= BLOCK_TRANSPARENCY_START))
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
						blocks_getVertexPosition(BLOCK_NEGATIVE_Z, l, &x, &y, &z);
						data |= (j-1 + lroundf(x)) & 0b111111u; data <<= 6;
						data |= (i-1 + lroundf(y)) & 0b111111u; data <<= 6;
						data |= (k-1 + lroundf(z)) & 0b111111u; data <<= 4;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_NEGATIVE_Z, l, &x, &y);
						data |= lroundf(10 * x) & 0b1111u; data <<= 4;
						data |= lroundf(10 * y) & 0b1111u; data <<= 3;
						data |= 0b10u;

						seqtor_push_back(verticesNormal, data);
					}
					currentVertex += 4;
				}

				//neg x
				if (chomk.blocks[i][j - 1][k] != chomk.blocks[i][j][k] && (chomk.blocks[i][j - 1][k] == BLOCK_AIR || chomk.blocks[i][j - 1][k] >= BLOCK_TRANSPARENCY_START))
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
						blocks_getVertexPosition(BLOCK_NEGATIVE_X, l, &x, &y, &z);
						data |= (j-1 + lroundf(x)) & 0b111111u; data <<= 6;
						data |= (i-1 + lroundf(y)) & 0b111111u; data <<= 6;
						data |= (k-1 + lroundf(z)) & 0b111111u; data <<= 4;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_NEGATIVE_X, l, &x, &y);
						data |= lroundf(10 * x) & 0b1111u; data <<= 4;
						data |= lroundf(10 * y) & 0b1111u; data <<= 3;
						data |= 0b11u;

						seqtor_push_back(verticesNormal, data);
					}
					currentVertex += 4;
				}

				//pos y
				if (chomk.blocks[i + 1][j][k] != chomk.blocks[i][j][k] && (chomk.blocks[i + 1][j][k] == BLOCK_AIR || chomk.blocks[i + 1][j][k] >= BLOCK_TRANSPARENCY_START))
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
						blocks_getVertexPosition(BLOCK_POSITIVE_Y, l, &x, &y, &z);
						data|= (j -1 + lroundf(x))& 0b111111u; data <<= 6;
						data|= (i -1 + lroundf(y))& 0b111111u; data <<= 6;
						data|= (k -1 + lroundf(z))& 0b111111u; data <<= 4;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_POSITIVE_Y, l, &x, &y);
						data|= lroundf(10 * x)& 0b1111u; data <<= 4;
						data|= lroundf(10 * y)& 0b1111u; data <<= 3;
						data |= 0b100u;

						seqtor_push_back(verticesNormal, data);
					}
					currentVertex += 4;
				}

				//neg y
				if (chomk.blocks[i - 1][j][k] != chomk.blocks[i][j][k] && (chomk.blocks[i - 1][j][k] == BLOCK_AIR || i != 0 && chomk.blocks[i - 1][j][k] >= BLOCK_TRANSPARENCY_START))
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
						blocks_getVertexPosition(BLOCK_NEGATIVE_Y, l, &x, &y, &z);
						data |= (j -1+ lroundf(x)) & 0b111111u; data <<= 6;
						data |= (i -1+ lroundf(y)) & 0b111111u; data <<= 6;
						data |= (k -1+ lroundf(z)) & 0b111111u; data <<= 4;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_NEGATIVE_Y, l, &x, &y);
						data |= lroundf(10 * x) & 0b1111u; data <<= 4;
						data |= lroundf(10 * y) & 0b1111u; data <<= 3;
						data |= 0b101u;

						seqtor_push_back(verticesNormal, data);
					}
					currentVertex += 4;
				}
			}
		}
	}


	//normal mesh
	if (verticesNormal.size > 0)
	{
		meshNormal->sizeVertices = verticesNormal.size * sizeof(unsigned long);
		meshNormal->vertices = (float*)malloc(meshNormal->sizeVertices);
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

	currentVertex = 0;//melyik az oldal elso csucsa (kell az indexeleshez)
	for (int i = 1; i < CHUNK_HEIGHT+1; i++)//y
	{
		for (int j = 1; j < CHUNK_WIDTH+1; j++)//x
		{
			for (int k = 1; k < CHUNK_WIDTH+1; k++)//z 
			{
				if (chomk.blocks[i][j][k] != BLOCK_WATER)
					continue;


				//pos y
				if (chomk.blocks[i + 1][j][k] != BLOCK_WATER)
				{
					//indices (6 per side (2 triangles))
					seqtor_push_back(indicesWalter, currentVertex);
					seqtor_push_back(indicesWalter, currentVertex + 2);
					seqtor_push_back(indicesWalter, currentVertex + 1);
					seqtor_push_back(indicesWalter, currentVertex + 3);
					seqtor_push_back(indicesWalter, currentVertex + 2);
					seqtor_push_back(indicesWalter, currentVertex);

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						unsigned long data = 0b0;
						blocks_getVertexPosition(BLOCK_POSITIVE_Y, l, &x, &y, &z);
						data |= (j -1+ lroundf(x)) & 0b111111u; data <<= 6;
						data |= (i -1+ lroundf(y)) & 0b111111u; data <<= 6;
						data |= (k -1+ lroundf(z)) & 0b111111u; data <<= 3;

						data |= 0b100u;

						seqtor_push_back(verticesWalter, data);

						seqtor_push_back(verticesWalter, 69);
						seqtor_push_back(verticesWalter, 69);
						*((float*)verticesWalter.data + verticesWalter.size - 2) = (j+x)/CHUNK_WIDTH;
						*((float*)verticesWalter.data + verticesWalter.size - 1) = (k+z)/CHUNK_WIDTH;
					}
					currentVertex += 4;
				}
			}
		}
	}

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

	//check for changedBlocks
	if (firstLoad)
	{
		if (current->size != 0)
		{
			changedBlocksInChunk cbic;
			cbic.chunkX = chunkX;
			cbic.chunkY = chunkY;
			cbic.chunkZ = chunkZ;
			cbic.blocks.data = current->data;
			cbic.blocks.capacity = current->capacity;
			cbic.blocks.size = current->size;

			seqtor_push_back(cm->changedBlocks, cbic);

			free(current);
		}
		else
		{
			seqtor_destroy(*current);
			free(current);
		}
	}

	return chomk;
}

void chunk_loadMeshInGPU(chunk* chomk, meshRaw meshNormal, meshRaw meshWalter)
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

		glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(unsigned long)+2*sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(unsigned long) + 2 * sizeof(float), (void*)(sizeof(unsigned long)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);

		chomk->waterMesh.indexCount = meshWalter.indexCount;
		chomk->isThereWaterMesh = 69;
	}
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