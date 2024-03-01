#ifndef COLLISION_DETECTION_H
#define COLLISION_DETECTION_H

#include "../collider/collider.h"

/*A collider_non_kinematic lastCollisionTag-je akkor allitodik at, ha az addigi ertek 0 vagy az utkozes ketto szilard test kozott van
* Ezt majd kesobb ki kene javitani
*/
int collisionDetection_collision(collider* collider_non_kinematic, collider* collider_kinematic);
#endif