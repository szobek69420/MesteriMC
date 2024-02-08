#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "../collider/collider.h"
#include "../collider_group/collider_group.h"

#include "../../utils/lista.h"

#include <pthread.h>

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

struct physicsSystem {
	lista_of(collider) simulatedColliders;//list of non kinematic colliders
	lista_of(colliderGroup) colliderGroups;
	lista_of(physicsSystemUpdate) pendingUpdates;
	pthread_mutex_t mutex_pending;
};
typedef struct physicsSystem physicsSystem;

physicsSystem physicsSystem_create();
void physicsSystem_destroy(physicsSystem* ps);

void physicsSystem_addGroup(physicsSystem* ps, colliderGroup cg);
void physicsSystem_removeGroup(physicsSystem* ps, int colliderGroupId);
colliderGroup* physicsSystem_getGroup(physicsSystem* ps, int colliderGroupId);

void physicsSystem_addCollider(physicsSystem* ps, collider c);
void physicsSystem_removeCollider(physicsSystem* ps, int colliderId);
collider* physicsSystem_getCollider(physicsSystem* ps, int colliderId);

void physicsSystem_processPending(physicsSystem* ps);

void physicsSystem_update(physicsSystem* ps, float deltaTime);

#endif