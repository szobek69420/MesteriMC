#include "blocks.h"
#include <stdio.h>

static float vertexPosition[];
static float vertexNormal[];
static float vertexTangent[];
static float vertexBitangent[];
static float vertexUV[];

void blocks_getVertexPosition(int side, int index, float* x, float* y, float* z)
{
	*x = vertexPosition[12 * side + 3 * index];
	*y = vertexPosition[12 * side + 3 * index + 1];
	*z = vertexPosition[12 * side + 3 * index + 2];
}

void blocks_getVertexNormal(int side, float* x, float* y, float* z)
{
	*x = vertexNormal[3*side];
	*y = vertexNormal[3*side+1];
	*z = vertexNormal[3*side+2];
}

void blocks_getVertexTangent(int side, float* x, float* y, float* z)
{
	*x = vertexTangent[3 * side];
	*y = vertexTangent[3 * side + 1];
	*z = vertexTangent[3 * side + 2];
}

void blocks_getVertexBitangent(int side, float* x, float* y, float* z)
{
	*x = vertexBitangent[3 * side];
	*y = vertexBitangent[3 * side + 1];
	*z = vertexBitangent[3 * side + 2];
}

void blocks_getUV(int block, int side, int index, float* uvx, float* uvy)
{
	int szam = 18 * block + 3 * side;
	*uvx = vertexUV[szam];
	*uvx= vertexUV[szam+1];
	
	//ez trukkos
	if (index % 3)
		*uvx += vertexUV[szam + 2];
	if (index / 2)
		*uvy += vertexUV[szam + 2];
}

//minden oldalhoz a bal also sarok uv-ja van mentve
static float vertexUV[] = {
	//air
	0,0,0,//uv x, uv y, szelesseg
	0,0,0,
	0,0,0,
	0,0,0,
	0,0,0,
	0,0,0,

	//water
	0.9,0,0.1,
	0.9,0,0.1,
	0.9,0,0.1,
	0.9,0,0.1,
	0.9,0,0.1,
	0.9,0,0.1,

	//stone
	0,0.9,0.1,
	0,0.9,0.1,
	0,0.9,0.1,
	0,0.9,0.1,
	0,0.9,0.1,
	0,0.9,0.1,

	//dirt
	0.1f, 0.9f, 0.1f,
	0.1f, 0.9f, 0.1f,
	0.1f, 0.9f, 0.1f,
	0.1f, 0.9f, 0.1f,
	0.1f, 0.9f, 0.1f,
	0.1f, 0.9f, 0.1f,

	//grass
	0.2f, 0.9f, 0.1f,
	0.2f, 0.9f, 0.1f,
	0.2f, 0.9f, 0.1f,
	0.2f, 0.9f, 0.1f,
	0.3f, 0.9f, 0.1f,
	0.1f, 0.9f, 0.1f
};

static float vertexPosition[] = {
	0,0,1,
	1,0,1,
	1,1,1,
	0,1,1,

	1,0,1,
	1,0,0,
	1,1,0,
	1,1,1,

	1,0,0,
	0,0,0,
	0,1,0,
	1,1,0,

	0,0,0,
	0,0,1,
	0,1,1,
	0,1,0,

	0,1,1,
	1,1,1,
	1,1,0,
	0,1,0,

	0,0,0,
	1,0,0,
	1,0,1,
	0,0,1
};

static float vertexNormal[] = {
	0,0,1,
	1,0,0,
	0,0,-1,
	-1,0,0,
	0,1,0,
	0,-1,0
};

static float vertexTangent[] = {
	1,0,0,
	0,0,-1,
	-1,0,0,
	0,0,1,
	1,0,0,
	1,0,0
};

static float vertexBitangent[] = {
	0,1,0,
	0,1,0,
	0,1,0,
	0,1,0,
	0,0,-1,
	0,0,1
};