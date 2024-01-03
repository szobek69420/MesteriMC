#ifndef CAMERA_H
#define CAMERA_H

#include "../glm2/vec3.h"
#include "../glm2/mat4.h"

typedef struct {
    vec3 position;
    vec3 front, up, right;
    vec3 world_up;
    float yaw, pitch;
    float fov;
    float move_speed, mouse_sensitivity;
} camera;

camera camera_create(vec3 position, vec3 world_up, float yaw, float pitch, float fov, float move_speed, float mouse_sensitivity);
void camera_update(camera* cam, float delta_time);

mat4 camera_getViewMatrix(camera* cam);

#endif