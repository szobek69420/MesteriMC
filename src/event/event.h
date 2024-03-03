#ifndef EVENT_H
#define EVENT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef enum {
    NONE = 0,
    WINDOW_RESIZE,
    KEY_PRESSED, KEY_RELEASED,
    MOUSE_BUTTON_PRESSED, MOUSE_BUTTON_RELEASED, MOUSE_MOVED, MOUSE_SCROLLED
} event_type;

typedef struct {
    int width, height;
} window_resize_event;

typedef struct {
    int key_code;
    int mods;
} key_pressed_event;

typedef struct {
    int key_code;
} key_released_event;

typedef struct {
    int button;
} mouse_button_pressed_event;

typedef struct {
    int button;
} mouse_button_released_event;

typedef struct {
    double x, y;
} mouse_moved_event;

typedef struct {
    double x, y;
} mouse_scrolled_event;

typedef struct {
    event_type type;
    union event_data
    {
        window_resize_event window_resize;
        key_pressed_event key_pressed;
        key_released_event key_released;
        mouse_button_pressed_event mouse_button_pressed;
        mouse_button_released_event mouse_button_released;
        mouse_moved_event mouse_moved;
        mouse_scrolled_event mouse_scrolled;
    } data;
} event;

#endif