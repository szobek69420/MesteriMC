#ifndef INPUT_H
#define INPUT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "event/event.h"

typedef struct {
    int previous_key_state[GLFW_KEY_LAST];
    int current_key_state[GLFW_KEY_LAST];
    int previous_mouse_button_state[GLFW_MOUSE_BUTTON_LAST];
    int current_mouse_button_state[GLFW_MOUSE_BUTTON_LAST];
    double previous_mouse_x, previous_mouse_y;
    double current_mouse_x, current_mouse_y;
    double mouse_scroll_delta_x, mouse_scroll_delta_y;
} input_data;

void input_init();
void input_update();
void input_handle_event(event e);

int input_is_key_pressed(int key);
int input_is_key_down(int key);
int input_is_key_released(int key);

int input_is_mouse_button_pressed(int button);
int input_is_mouse_button_down(int button);
int input_is_mouse_button_released(int button);

void input_get_mouse_position(double* x, double* y);
void input_get_mouse_scroll_delta(double* x, double* y);

#endif