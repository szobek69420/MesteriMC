#include "input.h"

#include <string.h>
#include <ctype.h>

static input_data input;

void input_init()
{
    memset(input.previous_key_state, 0, sizeof(int) * GLFW_KEY_LAST);
    memset(input.current_key_state, 0, sizeof(int) * GLFW_KEY_LAST);
    memset(input.previous_mouse_button_state, 0, sizeof(int) * GLFW_MOUSE_BUTTON_LAST);
    memset(input.current_mouse_button_state, 0, sizeof(int) * GLFW_MOUSE_BUTTON_LAST);
    input.previous_mouse_x = input.previous_mouse_y = 0.0;
    input.current_mouse_x = input.current_mouse_y = 0.0;
    input.mouse_scroll_delta_x = input.mouse_scroll_delta_y = 0.0;
}
void input_update()
{
    memcpy(input.previous_key_state, input.current_key_state, sizeof(int) * GLFW_KEY_LAST);
    memcpy(input.previous_mouse_button_state, input.current_mouse_button_state, sizeof(int) * GLFW_MOUSE_BUTTON_LAST);
    memset(input.key_modifiers, 0, sizeof(int) * GLFW_KEY_LAST);
    input.previous_mouse_x = input.current_mouse_x;
    input.previous_mouse_y = input.current_mouse_y;
    input.mouse_scroll_delta_x = input.mouse_scroll_delta_y = 0.0;
}
void input_handle_event(event e)
{
    switch (e.type)
    {
    case KEY_PRESSED:
        input.current_key_state[e.data.key_pressed.key_code] = 1;
        input.key_modifiers[e.data.key_pressed.key_code] = e.data.key_pressed.mods;
        break;
    case KEY_RELEASED:
        input.current_key_state[e.data.key_released.key_code] = 0;
        break;
    case MOUSE_BUTTON_PRESSED:
        input.current_mouse_button_state[e.data.mouse_button_pressed.button] = 1;
        break;
    case MOUSE_BUTTON_RELEASED:
        input.current_mouse_button_state[e.data.mouse_button_released.button] = 0;
        break;
    case MOUSE_MOVED:
        input.current_mouse_x = e.data.mouse_moved.x;
        input.current_mouse_y = e.data.mouse_moved.y;
        break;
    case MOUSE_SCROLLED:
        input.mouse_scroll_delta_x += e.data.mouse_scrolled.x;
        input.mouse_scroll_delta_y += e.data.mouse_scrolled.y;
        break;
    default:
        break;
    }
}

int input_is_key_pressed(int key)
{
    if (key >= GLFW_KEY_LAST || key < 0)
        return 0;
    return input.current_key_state[key] && !input.previous_key_state[key];
}
int input_is_key_down(int key)
{
    if (key >= GLFW_KEY_LAST || key < 0)
        return 0;
    return input.current_key_state[key];
}
int input_is_key_released(int key)
{
    if (key >= GLFW_KEY_LAST || key < 0)
        return 0;
    return !input.current_key_state[key] && input.previous_key_state[key];
}
int input_get_key_modifiers(int key)
{
    if (key >= GLFW_KEY_LAST || key < 0)
        return 0;
    return input.key_modifiers[key];
}
int input_is_key_capital(int key)
{
    if (key >= GLFW_KEY_LAST || key < 0)
        return 0;

    const char* temp = glfwGetKeyName(key, 0);
    if (temp == NULL || isalpha(temp[0]) == 0)
        return 0;

    int capital = 0;
    if (input.key_modifiers[key] & GLFW_MOD_SHIFT)
        capital = 1 - capital;
    if (input.key_modifiers[key] & GLFW_MOD_CAPS_LOCK)
        capital = 1 - capital;
    return capital;
}

int input_is_mouse_button_pressed(int button)
{
    if (button >= GLFW_MOUSE_BUTTON_LAST || button < 0)
        return 0;
    return input.current_mouse_button_state[button] && !input.previous_mouse_button_state[button];
}
int input_is_mouse_button_down(int button)
{
    if (button >= GLFW_MOUSE_BUTTON_LAST || button < 0)
        return 0;
    return input.current_mouse_button_state[button];
}
int input_is_mouse_button_released(int button)
{
    if (button >= GLFW_MOUSE_BUTTON_LAST || button < 0)
        return 0;
    return !input.current_mouse_button_state[button] && input.previous_mouse_button_state[button];
}


void input_get_mouse_position(double* x, double* y)
{
    *x = input.current_mouse_x;
    *y = input.current_mouse_y;
}
void input_get_mouse_delta(double* x, double* y)
{
    *x = input.current_mouse_x - input.previous_mouse_x;
    *y = input.current_mouse_y - input.previous_mouse_y;
}
void input_get_mouse_scroll_delta(double* x, double* y)
{
    *x = input.mouse_scroll_delta_x;
    *y = input.mouse_scroll_delta_y;
}