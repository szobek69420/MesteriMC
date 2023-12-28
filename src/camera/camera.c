#include "camera.h"

#include "../input/input.h"

#include <math.h>

#define DEG2RAD 0.01745329251

static void _camera_update_vectors(camera* cam);

camera camera_create(vec3 position, vec3 world_up, float yaw, float pitch, float fov, float move_speed, float mouse_sensitivity)
{
    camera cam;
    cam.position = position;
    cam.world_up = world_up;
    cam.yaw = yaw;
    cam.pitch = pitch;
    cam.fov = fov;
    cam.move_speed = move_speed;
    cam.mouse_sensitivity = mouse_sensitivity;
    _camera_update_vectors(&cam);
    return cam;
}
void camera_update(camera* cam, float delta_time)
{
    //keyboard
    float velocity = cam->move_speed * delta_time;
    if (input_is_key_down(GLFW_KEY_W))
        cam->position = vec3_sum(cam->position, vec3_scale(cam->front, velocity));
    if (input_is_key_down(GLFW_KEY_S))
        cam->position = vec3_sum(cam->position, vec3_scale(cam->front, -velocity));
    if (input_is_key_down(GLFW_KEY_A))
        cam->position = vec3_sum(cam->position, vec3_scale(cam->right, -velocity));
    if (input_is_key_down(GLFW_KEY_D))
        cam->position = vec3_sum(cam->position, vec3_scale(cam->right, velocity));

    //mouse movement
    double dx, dy;
    input_get_mouse_delta(&dx, &dy);
    dx *= cam->mouse_sensitivity;
    dy *= cam->mouse_sensitivity;
    cam->yaw -= dx;
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

mat4 camera_get_view_matrix(camera* cam)
{
    return mat4_lookAt(cam->position, cam->front, cam->up);
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
}