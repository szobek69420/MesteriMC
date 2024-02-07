#include "collision_detection.h"

#include <math.h>

#include "../collider/collider.h"

#include "../../glm2/vec3.h"

int collisionDetection_shouldResolve(collider* c1, collider* c2)
{
	int temp=0;
	switch (collider_getType(c1))
	{
	case COLLIDER_TYPE_BOX:
		switch (collider_getType(c2))
		{
		case COLLIDER_TYPE_BOX://box with box
			if (fabsf(c1->position.x - c2->position.x) > c1->box.sizePerTwo.x + c2->box.sizePerTwo.x)
				return 0;
			if (fabsf(c1->position.y - c2->position.y) > c1->box.sizePerTwo.y + c2->box.sizePerTwo.y)
				return 0;
			if (fabsf(c1->position.z - c2->position.z) > c1->box.sizePerTwo.z + c2->box.sizePerTwo.z)
				return 0;
			return 69;
		}
		break;
	}
}

void collisionDetection_collisionBoxBox(collider* cn, collider* ck)
{

}