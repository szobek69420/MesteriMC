#include "chunk.h"
#include "chunkManager.h"
#include "../blocks/blocks.h"
#include <stdlib.h>
#include <glad/glad.h>
#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"

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


chunk chunk_generate(chunkManager* cm, int chunkX, int chunkY, int chunkZ, meshRaw* meshNormal, meshRaw* meshWalter)
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
			heightHelper = fnlGetNoise2D(&(cm->noise), basedX + i, basedZ + j)*fnlGetNoise2D(&(cm->noise2),basedX+i, basedZ+j);
			heightHelper *= 200*heightHelper;
			heightHelper += 20;
			heightHelper -= basedY;
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

	//adding trees
	srand(1000000 * chunkX + 1000 * chunkY + chunkZ);
	for (int i = 2; i < CHUNK_WIDTH-2; i++)
	{
		for (int j = 2; j < CHUNK_WIDTH-2;j++)
		{
			int height = heightMap[i + 1][j + 1];
			if (height >= 0 && height < CHUNK_HEIGHT-6)
			{
				float szam = (float)rand() / RAND_MAX;
				if (chomk.blocks[height][i][j] == BLOCK_GRASS&&szam>0.995f)
				{
					for (int k = 0; k < sizeof(model_oak_tree) / sizeof(blockModel); k++)
					{
						blockModel bm = model_oak_tree[k];
						chomk.blocks[height + bm.y][i + bm.x][j + bm.z] = bm.type;
					}
				}
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
				if ((k == CHUNK_WIDTH - 1 && heightMap[j+1][CHUNK_WIDTH + 1]<i) || (k != CHUNK_WIDTH - 1 && chomk.blocks[i][j][k + 1] !=chomk.blocks[i][j][k]&&(chomk.blocks[i][j][k + 1] == BLOCK_AIR||  chomk.blocks[i][j][k + 1] >= BLOCK_TRANSPARENCY_START)))
					sideCount++;

				//pos x
				if ((j == CHUNK_WIDTH - 1 && heightMap[CHUNK_WIDTH + 1][k+1]<i) || (j!=CHUNK_WIDTH-1 && chomk.blocks[i][j + 1][k] != chomk.blocks[i][j][k] &&(chomk.blocks[i][j+1][k] == BLOCK_AIR||  chomk.blocks[i][j + 1][k] >= BLOCK_TRANSPARENCY_START)))
					sideCount++;

				//neg z
				if ((k == 0 && heightMap[j+1][0]<i) || (k!=0&& chomk.blocks[i][j][k - 1] != chomk.blocks[i][j][k] &&(chomk.blocks[i][j][k -1] == BLOCK_AIR|| chomk.blocks[i][j][k - 1] >= BLOCK_TRANSPARENCY_START)))
					sideCount++;

				//neg x
				if ((j == 0 &&heightMap[0][k+1]<i) || (j!=0&& chomk.blocks[i][j - 1][k] != chomk.blocks[i][j][k] &&(chomk.blocks[i][j - 1][k] == BLOCK_AIR|| chomk.blocks[i][j - 1][k] >= BLOCK_TRANSPARENCY_START)))
					sideCount++;

				//pos y
				if ((i == CHUNK_HEIGHT - 1 && heightMap[j+1][k+1] ==CHUNK_HEIGHT-1) || (i!=CHUNK_HEIGHT-1&& chomk.blocks[i + 1][j][k] != chomk.blocks[i][j][k] &&(chomk.blocks[i + 1][j][k] == BLOCK_AIR ||  chomk.blocks[i + 1][j][k] >= BLOCK_TRANSPARENCY_START)))
					sideCount++;

				//neg y
				if ((i == 0 &&basedY==0) || (i!=0&& chomk.blocks[i - 1][j][k] != chomk.blocks[i][j][k] &&(chomk.blocks[i - 1][j][k] == BLOCK_AIR || i != 0 && chomk.blocks[i - 1][j][k] >= BLOCK_TRANSPARENCY_START)))
					sideCount++;
			}
		}
	}

	if (sideCount == 0)
	{
		chomk.isThereNormalMesh = 0;
		goto WalterGeneration;
	}

	meshNormal->sizeVertices = 4 * sideCount * CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float);
	meshNormal->vertices = (float*)malloc(meshNormal->sizeVertices);

	meshNormal->sizeIndices = 6 * sideCount * sizeof(unsigned int);
	meshNormal->indices = (unsigned int*)malloc(meshNormal->sizeIndices);

	meshNormal->indexCount = 6 * sideCount;


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
				if ((k == CHUNK_WIDTH - 1 && heightMap[j + 1][CHUNK_WIDTH + 1] < i) || (k != CHUNK_WIDTH - 1 && chomk.blocks[i][j][k + 1] != chomk.blocks[i][j][k] && (chomk.blocks[i][j][k + 1] == BLOCK_AIR || chomk.blocks[i][j][k + 1] >= BLOCK_TRANSPARENCY_START)))
				{
					//indices (6 per side (2 triangles))
					meshNormal->indices[vertexIndexIndex++] = currentVertex;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 2;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 1;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 3;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 2;
					meshNormal->indices[vertexIndexIndex++] = currentVertex;

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						blocks_getVertexPosition(BLOCK_POSITIVE_Z, l, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = j + x;
						meshNormal->vertices[vertexDataIndex++] = i + y;
						meshNormal->vertices[vertexDataIndex++] = k + z;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_POSITIVE_Z, l, &x, &y);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						//meshNormal->vertices[vertexDataIndex++] = 0;//geometry

						blocks_getVertexNormal(BLOCK_POSITIVE_Z, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;

						blocks_getVertexTangent(BLOCK_POSITIVE_Z, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;

						/*blocks_getVertexBitangent(BLOCK_POSITIVE_Z, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;*/
					}

					currentVertex += 4;
				}

				//pos x
				if ((j == CHUNK_WIDTH - 1 && heightMap[CHUNK_WIDTH + 1][k + 1] < i) || (j != CHUNK_WIDTH - 1 && chomk.blocks[i][j + 1][k] != chomk.blocks[i][j][k] && (chomk.blocks[i][j + 1][k] == BLOCK_AIR || chomk.blocks[i][j + 1][k] >= BLOCK_TRANSPARENCY_START)))
				{
					//indices (6 per side (2 triangles))
					meshNormal->indices[vertexIndexIndex++] = currentVertex;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 2;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 1;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 3;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 2;
					meshNormal->indices[vertexIndexIndex++] = currentVertex;

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						blocks_getVertexPosition(BLOCK_POSITIVE_X, l, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = j + x;
						meshNormal->vertices[vertexDataIndex++] = i + y;
						meshNormal->vertices[vertexDataIndex++] = k + z;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_POSITIVE_X, l, &x, &y);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						//meshNormal->vertices[vertexDataIndex++] = 0;//geometry

						blocks_getVertexNormal(BLOCK_POSITIVE_X, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;

						blocks_getVertexTangent(BLOCK_POSITIVE_X, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;

						/*blocks_getVertexBitangent(BLOCK_POSITIVE_X, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;*/
					}
					currentVertex += 4;
				}

				//neg z
				if ((k == 0 && heightMap[j + 1][0] < i) || (k != 0 && chomk.blocks[i][j][k - 1] != chomk.blocks[i][j][k] && (chomk.blocks[i][j][k - 1] == BLOCK_AIR || chomk.blocks[i][j][k - 1] >= BLOCK_TRANSPARENCY_START)))
				{
					//indices (6 per side (2 triangles))
					meshNormal->indices[vertexIndexIndex++] = currentVertex;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 2;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 1;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 3;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 2;
					meshNormal->indices[vertexIndexIndex++] = currentVertex;

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						blocks_getVertexPosition(BLOCK_NEGATIVE_Z, l, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = j + x;
						meshNormal->vertices[vertexDataIndex++] = i + y;
						meshNormal->vertices[vertexDataIndex++] = k + z;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_NEGATIVE_Z, l, &x, &y);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						//meshNormal->vertices[vertexDataIndex++] = 0;//geometry

						blocks_getVertexNormal(BLOCK_NEGATIVE_Z, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;

						blocks_getVertexTangent(BLOCK_NEGATIVE_Z, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;

						/*blocks_getVertexBitangent(BLOCK_NEGATIVE_Z, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;*/
					}
					currentVertex += 4;
				}

				//neg x
				if ((j == 0 && heightMap[0][k + 1] < i) || (j != 0 && chomk.blocks[i][j - 1][k] != chomk.blocks[i][j][k] && (chomk.blocks[i][j - 1][k] == BLOCK_AIR || chomk.blocks[i][j - 1][k] >= BLOCK_TRANSPARENCY_START)))
				{
					//indices (6 per side (2 triangles))
					meshNormal->indices[vertexIndexIndex++] = currentVertex;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 2;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 1;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 3;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 2;
					meshNormal->indices[vertexIndexIndex++] = currentVertex;

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						blocks_getVertexPosition(BLOCK_NEGATIVE_X, l, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = j + x;
						meshNormal->vertices[vertexDataIndex++] = i + y;
						meshNormal->vertices[vertexDataIndex++] = k + z;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_NEGATIVE_X, l, &x, &y);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						//meshNormal->vertices[vertexDataIndex++] = 0;//geometry

						blocks_getVertexNormal(BLOCK_NEGATIVE_X, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;

						blocks_getVertexTangent(BLOCK_NEGATIVE_X, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;

						/*blocks_getVertexBitangent(BLOCK_NEGATIVE_X, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;*/
					}
					currentVertex += 4;
				}

				//pos y
				if ((i == CHUNK_HEIGHT - 1 && heightMap[j + 1][k + 1] == CHUNK_HEIGHT - 1) || (i != CHUNK_HEIGHT - 1 && chomk.blocks[i + 1][j][k] != chomk.blocks[i][j][k] && (chomk.blocks[i + 1][j][k] == BLOCK_AIR || chomk.blocks[i + 1][j][k] >= BLOCK_TRANSPARENCY_START)))
				{
					//indices (6 per side (2 triangles))
					meshNormal->indices[vertexIndexIndex++] = currentVertex;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 2;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 1;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 3;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 2;
					meshNormal->indices[vertexIndexIndex++] = currentVertex;

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						blocks_getVertexPosition(BLOCK_POSITIVE_Y, l, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = j + x;
						meshNormal->vertices[vertexDataIndex++] = i + y;
						meshNormal->vertices[vertexDataIndex++] = k + z;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_POSITIVE_Y, l, &x, &y);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						//meshNormal->vertices[vertexDataIndex++] = 0;//geometry

						blocks_getVertexNormal(BLOCK_POSITIVE_Y, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;

						blocks_getVertexTangent(BLOCK_POSITIVE_Y, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;

						/*blocks_getVertexBitangent(BLOCK_POSITIVE_Y, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;*/
					}
					currentVertex += 4;
				}

				//neg y
				if ((i == 0 && basedY == 0) || (i != 0 && chomk.blocks[i - 1][j][k] != chomk.blocks[i][j][k] && (chomk.blocks[i - 1][j][k] == BLOCK_AIR || i != 0 && chomk.blocks[i - 1][j][k] >= BLOCK_TRANSPARENCY_START)))
				{
					//indices (6 per side (2 triangles))
					meshNormal->indices[vertexIndexIndex++] = currentVertex;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 2;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 1;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 3;
					meshNormal->indices[vertexIndexIndex++] = currentVertex + 2;
					meshNormal->indices[vertexIndexIndex++] = currentVertex;

					//vertices (4 per side)
					for (int l = 0; l < 4; l++)
					{
						blocks_getVertexPosition(BLOCK_NEGATIVE_Y, l, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = j + x;
						meshNormal->vertices[vertexDataIndex++] = i + y;
						meshNormal->vertices[vertexDataIndex++] = k + z;

						blocks_getUV(chomk.blocks[i][j][k], BLOCK_NEGATIVE_Y, l, &x, &y);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						//meshNormal->vertices[vertexDataIndex++] = 0;//geometry

						blocks_getVertexNormal(BLOCK_NEGATIVE_Y, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;

						blocks_getVertexTangent(BLOCK_NEGATIVE_Y, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;

						/*blocks_getVertexBitangent(BLOCK_NEGATIVE_Y, &x, &y, &z);
						meshNormal->vertices[vertexDataIndex++] = x;
						meshNormal->vertices[vertexDataIndex++] = y;
						meshNormal->vertices[vertexDataIndex++] = z;*/
					}
					currentVertex += 4;
				}
			}
		}
	}


	chomk.isThereNormalMesh = 0;

WalterGeneration:

	chomk.isThereWaterMesh = 0;

	return chomk;
}

void chunk_loadMeshInGPU(chunk* chomk, meshRaw meshNormal, meshRaw meshWalter)
{
	if (meshNormal.indexCount == 0)
	{
		chomk->isThereNormalMesh = 0;
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

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);//positions
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);//uv and geom
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);//normal
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(3);//tangent
		/*glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(12 * sizeof(float)));
		glEnableVertexAttribArray(4);//bitangent*/

		glBindVertexArray(0);

		chomk->isThereNormalMesh = 69;
	}


	if (meshWalter.indexCount == 0)
	{
		chomk->isThereWaterMesh = 0;
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

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);//positions
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);//uv and geom
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);//normal
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(3);//tangent
		/*glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, CHUNK_NORMAL_MESH_VERTEX_SIZE * sizeof(float), (void*)(12 * sizeof(float)));
		glEnableVertexAttribArray(4);//bitangent*/

		glBindVertexArray(0);

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