#include "physics_system.h"

#include "../collider/collider.h"
#include "../collider_group/collider_group.h"

#include "../../utils/lista.h"

#include <stdlib.h>
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

void physicsSystem_addCollider(physicsSystem* ps, collider c)
{
	physicsSystemUpdate psu;
	psu.type = PHYSICS_ADD_COLLIDER;
	psu.c = c;
	lista_push_back(ps->pendingUpdates, psu);
}

void physicsSystem_removeCollider(physicsSystem* ps, int colliderId)
{
	physicsSystemUpdate psu;
	psu.type = PHYSICS_REMOVE_COLLIDER;
	psu.colliderId = colliderId;
	lista_push_back(ps->pendingUpdates, psu);
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