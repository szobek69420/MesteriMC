#ifndef COLLISION_DETECTION_H
#define COLLISION_DETECTION_H

#include "../collider/collider.h"

int collisionDetection_shouldResolve(collider* c1, collider* c2);

void collisionDetection_collisionBoxBox(collider* non_kinematic_box_collider, collider* kinematic_box_collider);

#endif