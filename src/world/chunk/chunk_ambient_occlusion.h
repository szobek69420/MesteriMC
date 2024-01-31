#ifndef CHUNK_AMBIENT_OCCLUSION_H
#define CHUNK_AMBIENT_OCCLUSION_H

#include "../blocks/blocks.h"

#define chunk_ao_switch(VAL, BUFFER) switch((VAL)) {\
	case 4: case 7: (BUFFER) = 0; break;\
	case 5: (BUFFER) = 1; break; \
	case 2: case 3: (BUFFER)=2; break;\
	default: (BUFFER) =3; break;\
}

//VNUM: vertex number, number of the vertex in one side of the block (0...3)
#define chunk_ao_pos_z(BLOCKS,X,Y,Z, VNUM, BUFFER) do { \
(BUFFER)=0; \
switch ((VNUM)) { \
	case 0: \
		if((BLOCKS)[(Y)][(X)-1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)-1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 1: \
		if((BLOCKS)[(Y)][(X)+1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)+1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 2: \
		if((BLOCKS)[(Y)][(X)+1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)+1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 3: \
		if((BLOCKS)[(Y)][(X)-1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)-1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
}\
chunk_ao_switch((BUFFER),(BUFFER)); \
} while(0)



#define chunk_ao_pos_x(BLOCKS,X,Y,Z, VNUM, BUFFER) do { \
(BUFFER)=0; \
switch ((VNUM)) { \
	case 0: \
		if((BLOCKS)[(Y)][(X)+1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)+1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)+1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 1: \
		if((BLOCKS)[(Y)][(X)+1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)+1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)+1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 2: \
		if((BLOCKS)[(Y)][(X)+1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)+1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)+1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 3: \
		if((BLOCKS)[(Y)][(X)+1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)+1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)+1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
}\
chunk_ao_switch((BUFFER),(BUFFER)); \
} while(0)


#define chunk_ao_neg_z(BLOCKS,X,Y,Z, VNUM, BUFFER) do { \
(BUFFER)=0; \
switch ((VNUM)) { \
	case 0: \
		if((BLOCKS)[(Y)][(X)+1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)+1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 1: \
		if((BLOCKS)[(Y)][(X)-1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)-1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 2: \
		if((BLOCKS)[(Y)][(X)-1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)-1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 3: \
		if((BLOCKS)[(Y)][(X)+1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)+1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
}\
chunk_ao_switch((BUFFER),(BUFFER)); \
} while(0)


#define chunk_ao_neg_x(BLOCKS,X,Y,Z, VNUM, BUFFER) do { \
(BUFFER)=0; \
switch ((VNUM)) { \
	case 0: \
		if((BLOCKS)[(Y)][(X)-1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)-1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)-1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 1: \
		if((BLOCKS)[(Y)][(X)-1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)-1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)-1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 2: \
		if((BLOCKS)[(Y)][(X)-1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)-1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)-1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 3: \
		if((BLOCKS)[(Y)][(X)-1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)-1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)-1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
}\
chunk_ao_switch((BUFFER),(BUFFER)); \
} while(0)



#define chunk_ao_pos_y(BLOCKS,X,Y,Z, VNUM, BUFFER) do { \
(BUFFER)=0; \
switch ((VNUM)) { \
	case 0: \
		if((BLOCKS)[(Y)+1][(X)][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)-1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)-1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 1: \
		if((BLOCKS)[(Y)+1][(X)][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)+1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)+1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 2: \
		if((BLOCKS)[(Y)+1][(X)][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)+1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)+1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 3: \
		if((BLOCKS)[(Y)+1][(X)][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)-1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)+1][(X)-1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
}\
chunk_ao_switch((BUFFER),(BUFFER)); \
} while(0)



#define chunk_ao_neg_y(BLOCKS,X,Y,Z, VNUM, BUFFER) do { \
(BUFFER)=0; \
switch ((VNUM)) { \
	case 0: \
		if((BLOCKS)[(Y)-1][(X)][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)-1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)-1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 1: \
		if((BLOCKS)[(Y)-1][(X)][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)+1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)+1][(Z)-1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 2: \
		if((BLOCKS)[(Y)-1][(X)][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)+1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)+1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
	case 3: \
		if((BLOCKS)[(Y)-1][(X)][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)-1][(Z)]!=BLOCK_AIR) \
			(BUFFER) += 2; \
		if((BLOCKS)[(Y)-1][(X)-1][(Z)+1]!=BLOCK_AIR) \
			(BUFFER) += 3; \
		break; \
}\
chunk_ao_switch((BUFFER),(BUFFER)); \
} while(0)

#endif