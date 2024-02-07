#include "collider.h"

#include "../../glm2/vec3.h"

static unsigned long colliderId=0;

struct aabb {
	vec3 sizePerTwo;
};
typedef struct aabb aabb;

struct sphere {
	float radius;
};
typedef struct sphere sphere;

struct collider {
	unsigned long id;
	unsigned long flags;
	/*
	* flags: [000000ed cccccccc bbbbbbbb aaaaaaaa]
	* a) type: 8 bits
	* b) tag: 8 bits
	* c) tag of the last collision: 8 bits
	* d) isKinematic: 1 bit
	* e) isSolid: 1 bit
	*/
	vec3 position;
	vec3 velocity;
	union {
		aabb box;
		sphere ball;
	};
};

collider collider_createBoxCollider(vec3 position, vec3 size, unsigned char isKinematic, unsigned char isSolid, unsigned short tag)
{
	collider c;
	c.id = colliderId++;

	c.flags = 0b0ul;
	c.flags |= 0b11111111ul & COLLIDER_TYPE_BOX;
	c.flags |= (0b11111111ul & tag) << 8;
	if (isKinematic != 0)
		c.flags |= 0b1000000000000000000000000ul;
	if(isSolid!=0)
		c.flags |= 0b10000000000000000000000000ul;

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
	c.flags |= 0b11111111ul & COLLIDER_TYPE_BOX;
	c.flags |= (0b11111111ul & tag) << 8;
	if (isKinematic != 0)
		c.flags |= 0b1000000000000000000000000ul;
	if (isSolid != 0)
		c.flags |= 0b10000000000000000000000000ul;

	c.position = position;
	c.ball.radius=radius;
	c.velocity = (vec3){ 0,0,0 };
	return c;
}

int collider_isKinematic(collider* c)
{
	if ((c->flags & 0b1000000000000000000000000ul) != 0)
		return 69;
	return 0;
}

int collider_isSolid(collider* c)
{
	if ((c->flags & 0b10000000000000000000000000ul) != 0)
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

