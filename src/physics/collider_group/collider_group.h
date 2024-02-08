#ifndef COLLIDER_GROUP_H
#define COLLIDER_GROUP_H

//kinematikus colliderek csoportja

#include "../../utils/seqtor.h"

struct colliderGroup {
	int id;

	//egy doboz, amely magaba foglalja az osszes collidert a colliderGroupban
	vec3 lowerBound;
	vec3 upperBound;

	seqtor_of(collider) colliders;
};

typedef struct colliderGroup colliderGroup;

colliderGroup colliderGroup_create(vec3 lowerBound, vec3 upperBound);
void colliderGroup_destroy(colliderGroup* cg);

void colliderGroup_addCollider(colliderGroup* cg, collider c);
void colliderGroup_removeCollider(colliderGroup* cg, int colliderId);

int colliderGroup_isColliderInBounds(colliderGroup* cg, collider* c);

#endif