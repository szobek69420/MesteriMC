#include "physics_system.h"

#include "../collider/collider.h"
#include "../collider_group/collider_group.h"
#include "../collision_detection/collision_detection.h"

#include "../../utils/seqtor.h"
#include "../../utils/lista.h"

#include <stdlib.h>
#include <math.h>
#include <pthread.h>

struct physicsSystem {
	seqtor_of(collider) simulatedColliders;//list of non kinematic colliders
	seqtor_of(colliderGroup) colliderGroups;
	lista_of(physicsSystemUpdate) pendingUpdates;
	pthread_mutex_t mutex_pending;
};

physicsSystem* physicsSystem_create()
{
	physicsSystem* ps=malloc(sizeof(physicsSystem));
	if (ps == NULL)
		return NULL;
	seqtor_init(ps->simulatedColliders,1);
	seqtor_init(ps->colliderGroups,1);
	lista_init(ps->pendingUpdates);
	pthread_mutex_init(&ps->mutex_pending, NULL);
	return ps;
}

void physicsSystem_destroy(physicsSystem* ps)
{
	seqtor_destroy(ps->simulatedColliders);


	for (int i = 0; i < seqtor_size(ps->colliderGroups); i++)
		colliderGroup_destroy(&seqtor_at(ps->colliderGroups, i));
	seqtor_destroy(ps->colliderGroups);


	physicsSystemUpdate psu;
	pthread_mutex_lock(&ps->mutex_pending);
	while (ps->pendingUpdates.size > 0)
	{
		lista_at(ps->pendingUpdates, 0, &psu);
		if (psu.type == PHYSICS_ADD_GROUP)
			colliderGroup_destroy(&psu.cg);
		lista_remove_at(ps->pendingUpdates, 0);
	}
	pthread_mutex_unlock(&ps->mutex_pending);

	pthread_mutex_destroy(&ps->mutex_pending);

	free(ps);
}

void physicsSystem_addGroup(physicsSystem* ps, colliderGroup cg)
{
	physicsSystemUpdate psu;
	psu.type = PHYSICS_ADD_GROUP;
	psu.cg = cg;

	pthread_mutex_lock(&ps->mutex_pending);
	lista_push_back(ps->pendingUpdates, psu);
	pthread_mutex_unlock(&ps->mutex_pending);
}

void physicsSystem_removeGroup(physicsSystem* ps, int colliderGroupId)
{
	physicsSystemUpdate psu;
	psu.type = PHYSICS_REMOVE_GROUP;
	psu.colliderGroupId = colliderGroupId;

	pthread_mutex_lock(&ps->mutex_pending);
	lista_push_back(ps->pendingUpdates, psu);
	pthread_mutex_unlock(&ps->mutex_pending);
}

colliderGroup* physicsSystem_getGroup(physicsSystem* ps, int colliderGroupId)
{
	for (int i = 0; i < seqtor_size(ps->colliderGroups); i++)
	{
		if (seqtor_at(ps->colliderGroups, i).id == colliderGroupId)
			return &seqtor_at(ps->colliderGroups, i);
	}
	return NULL;
}

void physicsSystem_addCollider(physicsSystem* ps, collider c)
{
	physicsSystemUpdate psu;
	psu.type = PHYSICS_ADD_COLLIDER;
	psu.c = c;
	lista_push(ps->pendingUpdates,0, psu);//azert a legelejere rakja, mert lehet, hogy amint hozza lesz adva egy collider, vissza is kell kerni a ra mutato mutatot es igy egy processPending hivassal be is lehet vinni a rendszerbe
}

void physicsSystem_removeCollider(physicsSystem* ps, int colliderId)
{
	physicsSystemUpdate psu;
	psu.type = PHYSICS_REMOVE_COLLIDER;
	psu.colliderId = colliderId;
	lista_push_back(ps->pendingUpdates, psu);
}

collider* physicsSystem_getCollider(physicsSystem* ps, int colliderId)
{
	for (int i = 0; i < seqtor_size(ps->simulatedColliders); i++)
	{
		if (seqtor_at(ps->simulatedColliders, i).id == colliderId)
			return &seqtor_at(ps->simulatedColliders, i);
	}
	return NULL;
}

void physicsSystem_processPending(physicsSystem* ps)
{
	if (ps->pendingUpdates.size == 0)
		return;

	physicsSystemUpdate psu;

	pthread_mutex_lock(&ps->mutex_pending);
	lista_at(ps->pendingUpdates, 0, &psu);
	lista_remove_at(ps->pendingUpdates, 0);
	pthread_mutex_unlock(&ps->mutex_pending);

	int index=0;
	lista_element_of(colliderGroup) * it1=NULL;
	lista_element_of(collider) * it2=NULL;
	switch (psu.type)
	{
	case PHYSICS_ADD_GROUP:
		seqtor_push_back(ps->colliderGroups,  psu.cg);
		break;

	case PHYSICS_REMOVE_GROUP:
		for (int i = 0; i < seqtor_size(ps->colliderGroups); i++)
		{
			if (seqtor_at(ps->colliderGroups, i).id == psu.colliderGroupId)
			{
				colliderGroup cg = seqtor_at(ps->colliderGroups, i);
				seqtor_at(ps->colliderGroups, i) = seqtor_at(ps->colliderGroups, seqtor_size(ps->colliderGroups) - 1);
				seqtor_remove_at(ps->colliderGroups, seqtor_size(ps->colliderGroups) - 1);
				colliderGroup_destroy(&cg);
				break;
			}
		}
		break;

	case PHYSICS_ADD_COLLIDER:
		seqtor_push_back(ps->simulatedColliders, psu.c);
		break;

	case PHYSICS_REMOVE_COLLIDER:
		for (int i = 0; i < seqtor_size(ps->simulatedColliders); i++)
		{
			if (seqtor_at(ps->simulatedColliders, i).id == psu.colliderId)
			{
				seqtor_at(ps->simulatedColliders, i) = seqtor_at(ps->simulatedColliders, seqtor_size(ps->simulatedColliders) - 1);
				seqtor_remove_at(ps->simulatedColliders, seqtor_size(ps->simulatedColliders) - 1);
				break;
			}
		}
		break;
	}
}

void physicsSystem_processPendingAll(physicsSystem* ps)
{
	pthread_mutex_lock(&ps->mutex_pending);
	int pendingCount = ps->pendingUpdates.size;
	pthread_mutex_unlock(&ps->mutex_pending);

	for (int i = 0; i < pendingCount; i++)
	{
		physicsSystem_processPending(ps);
	}
}

void physicsSystem_resolveCollisions(colliderGroup* cg, collider* c);

void physicsSystem_resetCollisions(physicsSystem* ps)
{
	for (int i=0; i<seqtor_size(ps->simulatedColliders); i++)
		COLLISION_RESET(seqtor_at(ps->simulatedColliders,i).flags);
}

void physicsSystem_update(physicsSystem* ps, float deltaTime)
{
	for (int i=0; i<seqtor_size(ps->simulatedColliders); i++)
	{
		seqtor_at(ps->simulatedColliders,i).position = vec3_sum(seqtor_at(ps->simulatedColliders, i).position, vec3_scale(seqtor_at(ps->simulatedColliders, i).velocity, deltaTime));

		for (int j=0; j<seqtor_size(ps->colliderGroups); j++)
		{
			if (colliderGroup_isColliderInBounds(&seqtor_at(ps->colliderGroups,j), &seqtor_at(ps->simulatedColliders,i)) == 0)
				continue;
			
			physicsSystem_resolveCollisions(&seqtor_at(ps->colliderGroups, j), &seqtor_at(ps->simulatedColliders, i));
		}
	}
}

//should be called, if a simulated collider is in a collider group
//currently it assumes that all of the colliders in cg are box colliders
void physicsSystem_resolveCollisions(colliderGroup* cg, collider* c)
{
	//obtain colliders that are in collision distance
	seqtor_of(collider) colliding;
	seqtor_init(colliding, 10);
	
	switch (collider_getType(c))
	{
	case COLLIDER_TYPE_BOX:
		for (int i = 0; i < seqtor_size(cg->colliders); i++)
		{
			float minDistanceX = c->box.sizePerTwo.x + seqtor_at(cg->colliders, i).box.sizePerTwo.x;
			float minDistanceY = c->box.sizePerTwo.y + seqtor_at(cg->colliders, i).box.sizePerTwo.y;
			float minDistanceZ = c->box.sizePerTwo.z + seqtor_at(cg->colliders, i).box.sizePerTwo.z;

			if (fabsf(c->position.x - seqtor_at(cg->colliders, i).position.x) > minDistanceX)
				continue;
			if (fabsf(c->position.y - seqtor_at(cg->colliders, i).position.y) > minDistanceY)
				continue;
			if (fabsf(c->position.z - seqtor_at(cg->colliders, i).position.z) > minDistanceZ)
				continue;
			seqtor_push_back(colliding, seqtor_at(cg->colliders, i));
		}
		break;

	case COLLIDER_TYPE_BALL:
		for (int i = 0; i < seqtor_size(cg->colliders); i++)
		{
			float minDistanceX = c->ball.radius + seqtor_at(cg->colliders, i).box.sizePerTwo.x;
			float minDistanceY = c->ball.radius + seqtor_at(cg->colliders, i).box.sizePerTwo.y;
			float minDistanceZ = c->ball.radius + seqtor_at(cg->colliders, i).box.sizePerTwo.z;

			if (fabsf(c->position.x - seqtor_at(cg->colliders, i).position.x) > minDistanceX)
				continue;
			if (fabsf(c->position.y - seqtor_at(cg->colliders, i).position.y) > minDistanceY)
				continue;
			if (fabsf(c->position.z - seqtor_at(cg->colliders, i).position.z) > minDistanceZ)
				continue;

			seqtor_push_back(colliding, seqtor_at(cg->colliders, i));
		}
		break;
	}

	//calculate distance between colliders and THE collider
	float* distances = calloc(colliding.size, sizeof(float));
	switch (collider_getType(c))
	{
	case COLLIDER_TYPE_BOX://manhattan distance, where in each direction the zero distance is when the two would be touching
		for (int i = 0; i < seqtor_size(colliding); i++)
		{
			float minDistanceX = c->box.sizePerTwo.x + seqtor_at(cg->colliders, i).box.sizePerTwo.x;
			float minDistanceY = c->box.sizePerTwo.y + seqtor_at(cg->colliders, i).box.sizePerTwo.y;
			float minDistanceZ = c->box.sizePerTwo.z + seqtor_at(cg->colliders, i).box.sizePerTwo.z;

			distances[i] += fabsf(c->position.x - seqtor_at(cg->colliders, i).position.x) - minDistanceX;
			distances[i] += fabsf(c->position.y - seqtor_at(cg->colliders, i).position.y) - minDistanceY;
			distances[i] += fabsf(c->position.z - seqtor_at(cg->colliders, i).position.z) - minDistanceZ;
		}
		break;

	case COLLIDER_TYPE_BALL:
		break;
	}

	//sort colliders nach distance in collision group in ascending order
	//bubble sort ftw
	for (int i = 0; i < (int)seqtor_size(colliding) - 1; i++)
	{
		for (int j = 0; j < (int)seqtor_size(colliding) - i - 1; j++)
		{
			if (distances[j] > distances[j + 1])
			{
				float temp = distances[j];
				distances[j] = distances[j + 1];
				distances[j + 1] = temp;

				collider temp2 = seqtor_at(colliding, j);
				seqtor_at(colliding, j) = seqtor_at(colliding, j + 1);
				seqtor_at(colliding, j + 1) = temp2;
			}
		}
	}

	free(distances);

	//resolve collisions in that order
	for (int i = 0; i < seqtor_size(colliding); i++)
	{
		collisionDetection_collision(c, &seqtor_at(colliding, i));
	}
	seqtor_destroy(colliding);
}

int physicsSystem_raycast(physicsSystem* ps, vec3 origin, vec3 direction, float distance, float precision, raycastHit* rh)
{
	collider raycaster = collider_createBoxCollider(origin, vec3_create(precision), 0, 1, 0);
	vec3 dir = vec3_scale(vec3_normalize(direction), precision);
	int stepCount = (int)(distance / precision);
	colliderGroup* currentCg=NULL;//save the current colliderGroup and reuse as long as the raycaster is in this group

	raycastHit rh2;

	for (int i = 0; i < stepCount; i++)
	{
		if (currentCg == NULL || colliderGroup_isColliderInBounds(currentCg, &raycaster) == 0)//ha meg nincsen vizsgalt collidergroup vagy az elozo mar elavult, akkor keres egy ujat
		{
			currentCg = NULL;
			for (int j=0; j<seqtor_size(ps->colliderGroups); j++)
			{
				if (colliderGroup_isColliderInBounds(&seqtor_at(ps->colliderGroups,j), &raycaster) == 0)
					continue;

				currentCg = &seqtor_at(ps->colliderGroups,j);
				break;
			}
		}
		if (currentCg == NULL)//nincs olyan collider group, amiben az aktualis mintavetelezes benne lenne
		{
			raycaster.position = vec3_sum(raycaster.position, dir);
			continue;
		}

		rh2.position = raycaster.position;
		physicsSystem_resolveCollisions(currentCg, &raycaster);
		if (COLLISION_HAS_COLLIDED(raycaster.flags))
			break;

		raycaster.position = vec3_sum(raycaster.position, dir);
	}

	if (COLLISION_HAS_COLLIDED(raycaster.flags))
	{
		if (rh != NULL)
		{
			rh2.tag = collider_getLastCollisionTag(&raycaster);

			if (COLLISION_GET_NEG_Z(raycaster.flags))
				rh2.normal = (vec3){ 0,0,1 };
			if (COLLISION_GET_NEG_X(raycaster.flags))
				rh2.normal = (vec3){ 1,0,0 };
			if (COLLISION_GET_POS_Z(raycaster.flags))
				rh2.normal = (vec3){ 0,0,-1 };
			if (COLLISION_GET_POS_X(raycaster.flags))
				rh2.normal = (vec3){ -1,0,0 };
			if (COLLISION_GET_POS_Y(raycaster.flags))
				rh2.normal = (vec3){ 0,-1,0 };
			if (COLLISION_GET_NEG_Y(raycaster.flags))
				rh2.normal = (vec3){ 0,1,0 };

			*rh = rh2;
		}

		return 69;
	}

	return 0;
}

int physicsSystem_getColliderCount(physicsSystem* ps)
{
	return seqtor_size(ps->simulatedColliders);
}

int physicsSystem_getColliderGroupCount(physicsSystem* ps)
{
	return seqtor_size(ps->colliderGroups);
}