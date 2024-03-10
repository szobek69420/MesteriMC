#include "camera.h"

#include "../input/input.h"

#include <math.h>

#define DEG2RAD 0.01745329251


camera camera_create(vec3 position, vec3 world_up, float move_speed, float mouse_sensitivity)
{
    camera cam;
    cam.position = position;
    cam.world_up = world_up;
    cam.yaw = 0;
    cam.pitch = 0;
    cam.fov = 60;
    cam.aspectXY = 1;
    cam.near_plane = 0.1;
    cam.far_plane = 300;
    cam.move_speed = move_speed;
    cam.mouse_sensitivity = mouse_sensitivity;
    camera_updateVectors(&cam);
    return cam;
}

void camera_setProjection(camera* cam, float fov, float aspectXY, float near_plane, float far_plane)
{
    cam->fov = fov;
    cam->near_plane = near_plane;
    cam->far_plane = far_plane;
    cam->aspectXY = aspectXY;
    cam->projection_matrix = mat4_perspective(cam->fov, cam->aspectXY, cam->near_plane, cam->far_plane);
}


mat4 camera_getViewMatrix(camera* cam)
{
    return cam->view_matrix;
}

void camera_updateVectors(camera* cam)
{
    vec3 front;
    front.x = -sinf(cam->yaw * DEG2RAD) * cosf(cam->pitch * DEG2RAD);
    front.y = sinf(cam->pitch * DEG2RAD);
    front.z = -cosf(cam->yaw * DEG2RAD) * cosf(cam->pitch * DEG2RAD);
    cam->front = vec3_normalize(front);
    cam->right = vec3_normalize(vec3_cross(cam->front, cam->world_up));
    cam->up = vec3_normalize(vec3_cross(cam->right, cam->front));
    cam->view_matrix = mat4_lookAt(cam->position, cam->front, cam->up);
}