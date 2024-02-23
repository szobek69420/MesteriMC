#include "collision_detection.h"

#include <math.h>

#include "../collider/collider.h"

#include "../../glm2/vec3.h"

int collisionDetection_collisionBoxBox(struct collider* cn, struct collider* ck);

int collisionDetection_collision(collider* c1, collider* c2)
{
	int temp=0;
	switch (collider_getType(c1))
	{
	case COLLIDER_TYPE_BOX:
		switch (collider_getType(c2))
		{
		case COLLIDER_TYPE_BOX://box with box
			return collisionDetection_collisionBoxBox(c1, c2);
		}
		break;
	}
}


int collisionDetection_collisionBoxBox(struct collider* cn, struct collider* ck)
{
	float minDistanceX = cn->box.sizePerTwo.x + ck->box.sizePerTwo.x;
	float minDistanceY = cn->box.sizePerTwo.y + ck->box.sizePerTwo.y;
	float minDistanceZ = cn->box.sizePerTwo.z + ck->box.sizePerTwo.z;

	//a cheap pre-check
	if (fabsf(cn->position.x - ck->position.x) > minDistanceX)
		return 0;
	if (fabsf(cn->position.y - ck->position.y) > minDistanceY)
		return 0;
	if (fabsf(cn->position.z - ck->position.z) > minDistanceZ)
		return 0;

	//resolve in the direction in which the penetration is the least

	float distanceX = fabsf(cn->position.x - ck->position.x);
	float distanceY = fabsf(cn->position.y - ck->position.y);
	float distanceZ = fabsf(cn->position.z - ck->position.z);

	float penetrations[3] = { minDistanceX - distanceX,minDistanceY - distanceY,minDistanceZ - distanceZ };
	int minIndex = 0;
	if (penetrations[1] < penetrations[0])
		minIndex = 1;
	if (penetrations[2] < penetrations[minIndex])
		minIndex = 2;

	int shouldBeResolved = collider_isSolid(cn) && collider_isSolid(ck);

	switch (minIndex)
	{
	case 0://x ist minimal
		if (cn->position.x > ck->position.x)
		{
			COLLISION_SET_NEG_X(cn->flags);//mert a non-kinematic-kal a negativ z iranybol utkozott a kinematic
			
			if (shouldBeResolved)
			{
				cn->position.x = ck->position.x + minDistanceX;
				cn->velocity.x = 0;
			}
		}
		else
		{
			COLLISION_SET_POS_X(cn->flags);

			if (shouldBeResolved)
			{
				cn->position.x = ck->position.x - minDistanceX;
				cn->velocity.x = 0;
			}
		}
		break;

	case 1://y ist minimal
		if (cn->position.y > ck->position.y)
		{
			COLLISION_SET_NEG_Y(cn->flags);
			
			if (shouldBeResolved)
			{
				cn->position.y = ck->position.y + minDistanceY;
				cn->velocity.y = 0;
			}
		}
		else
		{
			COLLISION_SET_POS_Y(cn->flags);
			
			if (shouldBeResolved)
			{
				cn->position.y = ck->position.y - minDistanceY;
				cn->velocity.y = 0;
			}
		}
		break;

	default://z ist minimal
		if (cn->position.z > ck->position.z)
		{
			COLLISION_SET_NEG_Z(cn->flags);
			
			if (shouldBeResolved)
			{
				cn->position.z = ck->position.z + minDistanceZ;
				cn->velocity.z = 0;
			}
		}
		else
		{
			COLLISION_SET_POS_Z(cn->flags);
			
			if (shouldBeResolved)
			{
				cn->position.z = ck->position.z - minDistanceZ;
				cn->velocity.z = 0;
			}
		}
		break;
	}

	return 69;
}