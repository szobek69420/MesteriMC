#include "collider.h"

#include <math.h>

#include "../../glm2/vec3.h"

static unsigned long colliderId=0;


collider collider_createBoxCollider(vec3 position, vec3 size, unsigned char isKinematic, unsigned char isSolid, unsigned short tag)
{
	collider c;
	c.id = colliderId++;

	c.flags = 0b0ul;
	c.flags |= 0b11111111ul & COLLIDER_TYPE_BOX;
	c.flags |= (0b11111111ul & tag) << 8;
	if (isKinematic != 0)
		c.flags |= 0b1000000000000000000000000000000ul;
	if(isSolid!=0)
		c.flags |= 0b10000000000000000000000000000000ul;

	c.position = position;
	c.box.sizePerTwo = vec3_scale(size, 0.5f);
	c.velocity = (vec3){ 0,0,0 };
	return c;
}

collider collider_createBallCollider(vec3 position, float radius, unsigned char isKinematic, unsigned char isSolid, unsigned short tag)
{
	collider c;
	c.id = colliderId++;
	
	c.flags = 0b0ul;
	c.flags |= 0b11111111ul & COLLIDER_TYPE_BALL;
	c.flags |= (0b11111111ul & tag) << 8;
	if (isKinematic != 0)
		c.flags |= 0b1000000000000000000000000000000ul;
	if (isSolid != 0)
		c.flags |= 0b10000000000000000000000000000000ul;

	c.position = position;
	c.ball.radius=radius;
	c.velocity = (vec3){ 0,0,0 };
	return c;
}

int collider_isKinematic(collider* c)
{
	if ((c->flags & 0b1000000000000000000000000000000ul) != 0)
		return 69;
	return 0;
}

void collider_setSolidity(collider* c, int isSolid)
{
	if (isSolid)
		c->flags |= 0b10000000000000000000000000000000ul;
	else
		c->flags &= 0b01111111111111111111111111111111ul;
}

int collider_isSolid(collider* c)
{
	if ((c->flags & 0b10000000000000000000000000000000ul) != 0)
		return 69;
	return 0;
}

int collider_getType(collider* c)
{
	return (c->flags & 0b11111111ul);
}

int collider_getTag(collider* c)
{
	return (c->flags >> 8) & 0b11111111ul;
}

void collider_setTag(collider* c, unsigned long tag)
{
	c->flags &= 0b11111111111111110000000011111111ul;
	c->flags |= (tag & 0b11111111ul) << 8;
}

int collider_getLastCollisionTag(collider* c)
{
	return (c->flags >> 16) & 0b11111111ul;
}

void collider_setLastCollisionTag(collider* c, unsigned long lastCollisionTag)
{
	c->flags &= 0b11111111000000001111111111111111ul;
	c->flags |= (lastCollisionTag & 0b11111111ul) << 16;
}

void collider_setPosition(collider* c, vec3 position)
{
	c->position = position;
}

vec3 collider_getPosition(collider* c)
{
	return c->position;
}

void collider_setVelocity(collider* c, vec3 velocity)
{
	c->velocity = velocity;
}

vec3 collider_getVelocity(collider* c)
{
	return c->velocity;
}

int collider_isInBounds(collider* c, vec3 lowerBound, vec3 upperBound)
{
	float minDistanceX = 0, minDistanceY = 0, minDistanceZ = 0;

	switch (collider_getType(c))
	{
	case COLLIDER_TYPE_BOX:
		minDistanceX = c->box.sizePerTwo.x + 0.5f * (upperBound.x - lowerBound.x);
		minDistanceY = c->box.sizePerTwo.y + 0.5f * (upperBound.y - lowerBound.y);
		minDistanceZ = c->box.sizePerTwo.z + 0.5f * (upperBound.z - lowerBound.z);
		break;

	case COLLIDER_TYPE_BALL:
		minDistanceX = c->ball.radius + 0.5f * (upperBound.x - lowerBound.x);
		minDistanceY = c->ball.radius + 0.5f * (upperBound.y - lowerBound.y);
		minDistanceZ = c->ball.radius + 0.5f * (upperBound.z - lowerBound.z);
		break;
	}

	//a cheap pre-check
	if (fabsf(c->position.x - 0.5f * (upperBound.x + lowerBound.x)) > minDistanceX)
		return 0;
	if (fabsf(c->position.y - 0.5f * (upperBound.y + lowerBound.y)) > minDistanceY)
		return 0;
	if (fabsf(c->position.z - 0.5f * (upperBound.z + lowerBound.z)) > minDistanceZ)
		return 0;

	return 69;
}
