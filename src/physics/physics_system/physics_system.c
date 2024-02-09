#include "physics_system.h"

#include "../collider/collider.h"
#include "../collider_group/collider_group.h"
#include "../collision_detection/collision_detection.h"

#include "../../utils/lista.h"

#include <stdlib.h>
#include <math.h>
#include <pthread.h>

physicsSystem physicsSystem_create()
{
	physicsSystem ps;
	lista_init(ps.simulatedColliders);
	lista_init(ps.colliderGroups);
	lista_init(ps.pendingUpdates);
	pthread_mutex_init(&ps.mutex_pending, NULL);
	return ps;
}

void physicsSystem_destroy(physicsSystem* ps)
{
	lista_clear(ps->simulatedColliders);

	colliderGroup cg;
	while (ps->colliderGroups.size > 0)
	{
		lista_at(ps->colliderGroups, 0, &cg);
		lista_remove_at(ps->colliderGroups, 0);
		colliderGroup_destroy(&cg);
	}

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
	for (lista_element_of(colliderGroup)* it = ps->colliderGroups.head; it != NULL; it = it->next)
	{
		if (it->data.id == colliderGroupId)
			return &it->data;
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
	for (lista_element_of(collider)* it = ps->simulatedColliders.head; it != NULL; it = it->next)
	{
		if (it->data.id == colliderId)
			return &it->data;
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
		lista_push_back(ps->colliderGroups,  psu.cg);
		break;

	case PHYSICS_REMOVE_GROUP:
		it1 = ps->colliderGroups.head;
		while (it1 != NULL)
		{
			if (it1->data.id == psu.colliderGroupId)
			{
				colliderGroup_destroy(&it1->data);
				lista_remove_at(ps->colliderGroups, index);
				break;
			}
			it1 = it1->next;
			index++;
		}
		break;

	case PHYSICS_ADD_COLLIDER:
		lista_push_back(ps->simulatedColliders, psu.c);
		break;

	case PHYSICS_REMOVE_COLLIDER:
		it2 = ps->simulatedColliders.head;
		while (it2 != NULL)
		{
			if (it2->data.id == psu.colliderId)
			{
				lista_remove_at(ps->simulatedColliders, index);
				break;
			}
			it2 = it2->next;
			index++;
		}
		break;
	}
}

void physicsSystem_resolveCollisions(colliderGroup* cg, collider* c);

void physicsSystem_update(physicsSystem* ps, float deltaTime)
{
	for (lista_element_of(collider)* it1 = ps->simulatedColliders.head; it1 != NULL; it1 = it1->next)
	{
		it1->data.position = vec3_sum(it1->data.position, vec3_scale(it1->data.velocity, deltaTime));

		for (lista_element_of(colliderGroup)* it2 = ps->colliderGroups.head; it2 != NULL; it2 = it2->next)
		{
			if (colliderGroup_isColliderInBounds(&it2->data, &it1->data) == 0)
				continue;
			
			physicsSystem_resolveCollisions(&it2->data, &it1->data);
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