#include "collider_group.h"

#include "../collider/collider.h"

#include "../../utils/seqtor.h"

#include "../../glm2/vec3.h"

static unsigned int colliderGroupId = 1;

colliderGroup colliderGroup_create(vec3 lowerBound, vec3 upperBound)
{
	colliderGroup cg;

	cg.id = colliderGroupId++;

	cg.lowerBound = lowerBound;
	cg.upperBound = upperBound;

	seqtor_init(cg.colliders, 1);

	return cg;
}

void colliderGroup_destroy(colliderGroup* cg)
{
	seqtor_destroy(cg->colliders);
}

void colliderGroup_addCollider(colliderGroup* cg, collider c)
{
	seqtor_push_back(cg->colliders, c);
}

void colliderGroup_removeCollider(colliderGroup* cg, int colliderId)
{
	int index = -1;
	for (int i = 0; i < seqtor_size(cg->colliders); i++)
	{
		if (seqtor_at(cg->colliders, i).id == colliderId)
		{
			index = i;
			break;
		}
	}
	
	if(index!=-1)
		seqtor_remove_at(cg->colliders, index);
}

int colliderGroup_isColliderInBounds(colliderGroup* cg, collider* c)
{
	switch (collider_getType(c))
	{
	case COLLIDER_TYPE_BOX:
		if (c->position.x + c->box.sizePerTwo.x < cg->lowerBound.x)
			return 0;
		if (c->position.x - c->box.sizePerTwo.x > cg->upperBound.x)
			return 0;
		if (c->position.y + c->box.sizePerTwo.y < cg->lowerBound.y)
			return 0;
		if (c->position.y - c->box.sizePerTwo.y > cg->upperBound.y)
			return 0;
		if (c->position.z + c->box.sizePerTwo.z < cg->lowerBound.z)
			return 0;
		if (c->position.z - c->box.sizePerTwo.z > cg->upperBound.z)
			return 0;
		return 69;

	case COLLIDER_TYPE_BALL://good enough
		if (c->position.x + c->ball.radius < cg->lowerBound.x)
			return 0;
		if (c->position.x - c->ball.radius > cg->upperBound.x)
			return 0;
		if (c->position.y + c->ball.radius < cg->lowerBound.y)
			return 0;
		if (c->position.y - c->ball.radius > cg->upperBound.y)
			return 0;
		if (c->position.z + c->ball.radius < cg->lowerBound.z)
			return 0;
		if (c->position.z - c->ball.radius > cg->upperBound.z)
			return 0;
		return 69;
	}
}