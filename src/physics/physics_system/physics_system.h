#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "../collider/collider.h"
#include "../collider_group/collider_group.h"

#include "../../utils/lista.h"

#define GRAVITY -25.0

#define PHYSICS_ADD_GROUP 1
#define PHYSICS_REMOVE_GROUP 2
#define PHYSICS_ADD_COLLIDER 3
#define PHYSICS_REMOVE_COLLIDER 4

struct physicsSystemUpdate {
	int type;//add or remove
	union {
		colliderGroup cg;
		int colliderGroupId;//id is used if removal is pending
		
		collider c;
		int colliderId;
	};
};
typedef struct physicsSystemUpdate physicsSystemUpdate;

struct physicsSystem;
typedef struct physicsSystem physicsSystem;

struct raycastHit {
	vec3 position;
	vec3 normal;
	unsigned int tag;
};
typedef struct raycastHit raycastHit;

struct physicsSystem* physicsSystem_create();
void physicsSystem_destroy(physicsSystem* ps);

void physicsSystem_addGroup(physicsSystem* ps, colliderGroup cg);
void physicsSystem_removeGroup(physicsSystem* ps, int colliderGroupId);
colliderGroup* physicsSystem_getGroup(physicsSystem* ps, int colliderGroupId);

void physicsSystem_addCollider(physicsSystem* ps, collider c);
void physicsSystem_removeCollider(physicsSystem* ps, int colliderId);
collider* physicsSystem_getCollider(physicsSystem* ps, int colliderId);

void physicsSystem_processPending(physicsSystem* ps);
void physicsSystem_processPendingAll(physicsSystem* ps);//processes all of the pending updates that exist at the time of calling

void physicsSystem_resetCollisions(physicsSystem* ps);
void physicsSystem_update(physicsSystem* ps, float deltaTime);

int physicsSystem_raycast(physicsSystem* ps, vec3 origin, vec3 direction, float distance, float precision, raycastHit* rh);//return value is zero, if nothing has been hit

int physicsSystem_getColliderCount(physicsSystem* ps);
int physicsSystem_getColliderGroupCount(physicsSystem* ps);

#endif