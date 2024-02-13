#ifndef CAMERA_H
#define CAMERA_H

#include "../glm2/vec3.h"
#include "../glm2/mat4.h"

typedef struct {
    vec3 position;
    vec3 front, up, right;
    vec3 world_up;
    float yaw, pitch;
    float fov, aspectXY;
    float near_plane, far_plane;
    float move_speed, mouse_sensitivity;
    mat4 view_matrix;
    mat4 projection_matrix;
} camera;

camera camera_create(vec3 position, vec3 world_up, float move_speed, float mouse_sensitivity);
void camera_setProjection(camera* cam, float fov, float aspectXY, float near_plane, float far_plane);
void camera_updateVectors(camera* cam);

mat4 camera_getViewMatrix(camera* cam);

#endif