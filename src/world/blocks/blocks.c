#include "blocks.h"

float uv[];

void blocks_getUV(int block, int side, int index, float* uvx, float* uvy)
{
	int szam = 18 * block + 3 * side;
	*uvx = uv[szam];
	*uvx= uv[szam+1];
	
	//ez trukkos
	if (index % 3)
		*uvx += uv[szam + 2];
	if (index / 2)
		*uvy += uv[szam + 2];
}

//minden oldalhoz a bal also sarok uv-ja van mentve
static float uv[] = {
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