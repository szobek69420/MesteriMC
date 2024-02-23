#ifndef COLLIDER_H
#define COLLIDER_H

#include "../../glm2/vec3.h"

#define COLLIDER_TYPE_BOX 1ul
#define COLLIDER_TYPE_BALL 2ul

#define COLLISION_HAS_COLLIDED(COLLIDER_FLAGS) (COLLIDER_FLAGS & 0b00111111000000000000000000000000ul)
#define COLLISION_RESET(COLLIDER_FLAGS) COLLIDER_FLAGS &= 0b11000000111111111111111111111111ul

#define COLLISION_SET_POS_Z(COLLIDER_FLAGS) COLLIDER_FLAGS |= 0b00000001000000000000000000000000ul
#define COLLISION_SET_POS_X(COLLIDER_FLAGS) COLLIDER_FLAGS |= 0b00000010000000000000000000000000ul
#define COLLISION_SET_NEG_Z(COLLIDER_FLAGS) COLLIDER_FLAGS |= 0b00000100000000000000000000000000ul
#define COLLISION_SET_NEG_X(COLLIDER_FLAGS) COLLIDER_FLAGS |= 0b00001000000000000000000000000000ul
#define COLLISION_SET_POS_Y(COLLIDER_FLAGS) COLLIDER_FLAGS |= 0b00010000000000000000000000000000ul
#define COLLISION_SET_NEG_Y(COLLIDER_FLAGS) COLLIDER_FLAGS |= 0b00100000000000000000000000000000ul

#define COLLISION_GET_POS_Z(COLLIDER_FLAGS) (COLLIDER_FLAGS & 0b00000001000000000000000000000000ul)
#define COLLISION_GET_POS_X(COLLIDER_FLAGS) (COLLIDER_FLAGS & 0b00000010000000000000000000000000ul)
#define COLLISION_GET_NEG_Z(COLLIDER_FLAGS) (COLLIDER_FLAGS & 0b00000100000000000000000000000000ul)
#define COLLISION_GET_NEG_X(COLLIDER_FLAGS) (COLLIDER_FLAGS & 0b00001000000000000000000000000000ul)
#define COLLISION_GET_POS_Y(COLLIDER_FLAGS) (COLLIDER_FLAGS & 0b00010000000000000000000000000000ul)
#define COLLISION_GET_NEG_Y(COLLIDER_FLAGS) (COLLIDER_FLAGS & 0b00100000000000000000000000000000ul)

struct collider_internal_aabb {
	vec3 sizePerTwo;
};

struct collider_internal_sphere {
	float radius;
};

struct collider {
	unsigned int id;
	unsigned int flags;
	/*
	* flags: [fedddddd cccccccc bbbbbbbb aaaaaaaa]
	* a) type: 8 bits
	* b) tag: 8 bits
	* c) tag of the last collision: 8 bits
	* d1) last collision from posZ: 1 bit
	* d2) last collision from posX: 1 bit
	* d3) last collision from negZ: 1 bit
	* d4) last collision from negX: 1 bit
	* d5) last collision from posY: 1 bit
	* d6) last collision from negY: 1 bit
	* e) isKinematic: 1 bit
	* f) isSolid: 1 bit
	*/
	vec3 position;
	vec3 velocity;
	union {
		struct collider_internal_aabb box;
		struct collider_internal_sphere ball;
	};
};

typedef struct collider collider;

collider collider_createBoxCollider(vec3 position, vec3 size, unsigned char isKinematic, unsigned char isSolid, unsigned short tag);
collider collider_createBallCollider(vec3 position, float radius, unsigned char isKinematic, unsigned char isSolid, unsigned short tag);

int collider_isKinematic(collider* c);

void collider_setSolidity(collider* c, int isSolid);
int collider_isSolid(collider* c);

int collider_getType(collider* c);

int collider_getTag(collider* c);
void collider_setTag(collider* c, unsigned long tag);

int collider_getLastCollisionTag(collider* c);
void collider_setLastCollisionTag(collider* c, unsigned long lastCollisionTag);

void collider_setPosition(collider* c, vec3 position);
vec3 collider_getPosition(collider* c);

void collider_setVelocity(collider* c, vec3 velocity);
vec3 collider_getVelocity(collider* c);

int collider_isInBounds(collider* c, vec3 lowerBound, vec3 upperBound);

#endif