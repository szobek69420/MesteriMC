#ifndef BLOCKS_H
#define BLOCKS_H

//block sides
#define BLOCK_POSITIVE_Z 0
#define BLOCK_POSITIVE_X 1
#define BLOCK_NEGATIVE_Z 2
#define BLOCK_NEGATIVE_X 3
#define BLOCK_POSITIVE_Y 4
#define BLOCK_NEGATIVE_Y 5

//block types
#define BLOCK_AIR 0
#define BLOCK_STONE 1
#define BLOCK_DIRT 2
#define BLOCK_GRASS 3
#define BLOCK_SAND 4
#define BLOCK_OAK_LOG 5
#define BLOCK_SUS 6
#define BLOCK_BORSOD 7
//from here only transparent blocks
#define BLOCK_TRANSPARENCY_START 8
#define BLOCK_WATER BLOCK_TRANSPARENCY_START
#define BLOCK_OAK_LEAVES (BLOCK_TRANSPARENCY_START+1)

typedef struct blockModel {
	int x, y, z;
	int type;//blocktype
} blockModel;

const char* blocks_getBlockName(int block);

//block: blokktipus, side: melyik oldal, index: az oldalon belul hanyadik csucs (4 csucs per oldal)
void blocks_getUV(int block, int side, int index, float* uvx, float* uvy);

void blocks_getVertexPosition(int side, int index, float* x, float* y, float* z);
void blocks_getVertexNormal(int side, float* x, float* y, float* z);
void blocks_getVertexTangent(int side, float* x, float* y, float* z);
void blocks_getVertexBitangent(int side, float* x, float* y, float* z);

#endif