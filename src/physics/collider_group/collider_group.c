#include "collider_group.h"

#include "../collider/collider.h"

#include "../../utils/seqtor.h"

#include "../../glm2/vec3.h"


colliderGroup colliderGroup_create(int id1, int id2, int id3, vec3 lowerBound, vec3 upperBound)
{
	colliderGroup cg;

	cg.id1 = id1;
	cg.id2 = id2;
	cg.id3 = id3;

	cg.lowerBound = lowerBound;
	cg.upperBound = upperBound;

	seqtor_init(cg.colliders, 1);
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