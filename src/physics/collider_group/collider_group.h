#ifndef COLLIDER_GROUP_H
#define COLLIDER_GROUP_H

//kinematikus colliderek csoportja

#include "../../utils/seqtor.h"

struct colliderGroup {
	int id1, id2, id3;//chunk szam lesz valszeg

	//egy doboz, amely magaba foglalja az osszes collidert a colliderGroupban
	vec3 lowerBound;
	vec3 upperBound;

	seqtor_of(collider) colliders;
};

typedef struct colliderGroup colliderGroup;

#endif