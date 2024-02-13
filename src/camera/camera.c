#include "camera.h"

#include "../input/input.h"

#include <math.h>

#define DEG2RAD 0.01745329251

static void _camera_update_vectors(camera* cam);

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
    _camera_update_vectors(&cam);
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

void camera_update(camera* cam, float delta_time)
{
    //keyboard (it is no longer updated here)
    float velocity = cam->move_speed * delta_time;
    vec3 forward = vec3_normalize(vec3_create2(cam->front.x, 0, cam->front.z));
    if (input_is_key_down(GLFW_KEY_W))
        cam->position = vec3_sum(cam->position, vec3_scale(forward, velocity));
    if (input_is_key_down(GLFW_KEY_S))
        cam->position = vec3_sum(cam->position, vec3_scale(forward, -velocity));
    if (input_is_key_down(GLFW_KEY_A))
        cam->position = vec3_sum(cam->position, vec3_scale(cam->right, -velocity));
    if (input_is_key_down(GLFW_KEY_D))
        cam->position = vec3_sum(cam->position, vec3_scale(cam->right, velocity));
    if (input_is_key_down(GLFW_KEY_LEFT_SHIFT))
        cam->position = vec3_sum(cam->position, vec3_create2(0,-velocity,0));
    if (input_is_key_down(GLFW_KEY_SPACE))
        cam->position = vec3_sum(cam->position, vec3_create2(0, velocity, 0));

    //mouse movement
    double dx, dy;
    input_get_mouse_delta(&dx, &dy);
    dx *= cam->mouse_sensitivity;
    dy *= cam->mouse_sensitivity;
    cam->yaw -= dx;
    if (cam->yaw > 180.0f)
        cam->yaw -= 360.0f;
    if (cam->yaw < -180.0f)
        cam->yaw += 360.0f;

    cam->pitch -= dy;
    if (cam->pitch > 89.0f)
        cam->pitch = 89.0f;
    if (cam->pitch < -89.0f)
        cam->pitch = -89.0f;
    _camera_update_vectors(cam);

    //mouse scroll
    input_get_mouse_scroll_delta(&dx, &dy);
    cam->fov -= dy;
    if (cam->fov < 1.0f)
        cam->fov = 1.0f;
    if (cam->fov > 179.0f)
        cam->fov = 179.0f;
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

static void _camera_update_vectors(camera* cam)
{
    vec3 front;
    front.x = -sinf(cam->yaw * DEG2RAD) * cosf(cam->pitch * DEG2RAD);
    front.y =  sinf(cam->pitch * DEG2RAD);
    front.z = -cosf(cam->yaw * DEG2RAD) * cosf(cam->pitch * DEG2RAD);
    cam->front = vec3_normalize(front);
    cam->right = vec3_normalize(vec3_cross(cam->front, cam->world_up));
    cam->up = vec3_normalize(vec3_cross(cam->right, cam->front));
    cam->view_matrix = mat4_lookAt(cam->position, cam->front, cam->up);
}