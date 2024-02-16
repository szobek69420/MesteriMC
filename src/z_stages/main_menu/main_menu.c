#define _CRT_SECURE_NO_WARNINGS
#include "main_menu.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "../stages.h"

#include "../../texture_handler/texture_handler.h"

#include "../../window/window.h"

#include "../../ui/canvas/canvas.h"
#include "../../ui/font_handler/font_handler.h"

#include "../../event/event.h"
#include "../../event/event_queue.h"
#include "../../input/input.h"

#ifdef sleep(DURATION_IN_SEX)
#undef sleep(DURATION_IN_SEX)
#endif
#ifdef _WIN32
#include <Windows.h>
#define sleep(DURATION_IN_SEX) do{ \
Sleep(1000*DURATION_IN_SEX); \
} while (0)
#elif defined(__unix__)
#include <unistd.h>
#else
#define sleep(DURATION) do{ \
float ____time = glfwGetTime(); \
while(glfwGetTime()-____time<DURATION) ; \
} while(0)
#endif

//global variables
GLFWwindow* w;
int* currentStage;

canvas* vaszon;
font f;
int background;

//glfw callbacks
void window_size_callback2(GLFWwindow* window, int width, int height);
void key_callback2(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback2(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback2(GLFWwindow* window, double xpos, double ypos);
void scroll_callback2(GLFWwindow* window, double xoffset, double yoffset);

//button callbacks
void startButton(void* currentStage);
void settingsButton(void* param);
void quitButton(void* currentStage);

//other function prototypes
void init();
void end();
void background_resize();
void handle_event2(event e);

//everything else
void mainMenu(void* window, int* _currentStage)
{
    w = (GLFWwindow*)window;
    currentStage = _currentStage;
    init();

    while (*currentStage == STAGE_MAIN_MENU)
    {
        //input update
        glfwPollEvents();
        input_update();
        event e;
        while ((e = event_queue_poll()).type != NONE)
            handle_event2(e);

        double x, y;
        glfwGetCursorPos(w, &x, &y);

        canvas_checkMouseInput(vaszon, x, y, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT));
        canvas_render(vaszon, x, y, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT));

        glfwSwapBuffers(w);

        if (glfwWindowShouldClose(w))
            *currentStage = STAGE_QUIT;

        sleep(0.05);
    }

    end();
}

void init()
{
    glfwSetWindowShouldClose(w, 0);

    glfwSetWindowSizeCallback(w, window_size_callback2);
    glfwSetKeyCallback(w, key_callback2);
    glfwSetMouseButtonCallback(w, mouse_button_callback2);
    glfwSetCursorPosCallback(w, NULL);
    glfwSetScrollCallback(w, NULL);
    glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    event_queue_init();
    input_init();

    textureHandler_importTextures(TEXTURE_MAIN_MENU);

    fontHandler_init();
    vaszon = canvas_create(window_getWidth(), window_getHeight(), "../assets/fonts/Monocraft.ttf");

    //filling canvas
    background = canvas_addImage(vaszon, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, 0, 0, 0, textureHandler_getTexture(TEXTURE_MENU_BACKGROUND));
    background_resize();
    canvas_addImage(vaszon, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_TOP, 0, 100, 700, 100, textureHandler_getTexture(TEXTURE_MENU_TITLE));

    int temp;
    temp = canvas_addButton(vaszon, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, 0, 400, 60);
    canvas_setButtonText(vaszon, temp, "mom's spaghetti", 24, 1, 1, 1);
    canvas_setButtonBorder(vaszon, temp, 5, 20);
    canvas_setButtonFillColour(vaszon, temp, 0.5, 0.5, 0.5);
    canvas_setButtonBorderColour(vaszon, temp, 0.3, 0.3, 0.3);
    canvas_setButtonClicked(vaszon, temp, startButton, currentStage);

    temp = canvas_addButton(vaszon, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, -70, 400, 60);
    canvas_setButtonText(vaszon, temp, "he keeps on adjustin'", 24, 1, 1, 1);
    canvas_setButtonBorder(vaszon, temp, 5, 20);
    canvas_setButtonFillColour(vaszon, temp, 0.5, 0.5, 0.5);
    canvas_setButtonBorderColour(vaszon, temp, 0.3, 0.3, 0.3);
    canvas_setButtonClicked(vaszon, temp, settingsButton, NULL);

    temp = canvas_addButton(vaszon, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, -140, 400, 60);
    canvas_setButtonText(vaszon, temp, "step back to reality", 24, 1, 1, 1);
    canvas_setButtonBorder(vaszon, temp, 5, 20);
    canvas_setButtonFillColour(vaszon, temp, 0.5, 0.5, 0.5);
    canvas_setButtonBorderColour(vaszon, temp, 0.3, 0.3, 0.3);
    canvas_setButtonClicked(vaszon, temp, quitButton, currentStage);
}

void end()
{
    fontHandler_close();
    fontHandler_destroyFont(&f);
    canvas_destroy(vaszon);
    textureHandler_destroyTextures(TEXTURE_MAIN_MENU);
}


void background_resize()
{
    float bgAspectXY;
    int w, h;
    glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_MENU_BACKGROUND));
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    bgAspectXY = (float)w / h;

    if (bgAspectXY > window_getAspect())
    {
        canvas_setComponentSize(vaszon, background, window_getHeight() * bgAspectXY, window_getHeight());
    }
    else
    {
        canvas_setComponentSize(vaszon, background, window_getWidth(), window_getWidth() / bgAspectXY);
    }
}

void handle_event2(event e)
{
    switch (e.type)
    {
    case WINDOW_RESIZE:
        window_setWidth(e.data.window_resize.width);
        window_setHeight(e.data.window_resize.height);
        glViewport(0, 0, window_getWidth(), window_getHeight());
        canvas_setSize(vaszon, window_getWidth(), window_getHeight());
        background_resize();
        break;
    default:
        input_handle_event(e);
        break;
    }
}


void window_size_callback2(GLFWwindow* window, int width, int height)
{
    if (event_queue_back().type != WINDOW_RESIZE)
        event_queue_push((event) { .type = WINDOW_RESIZE, .data.window_resize = { width, height } });
    else
        event_queue_swap_back((event) { .type = WINDOW_RESIZE, .data.window_resize = { width, height } });
}
void key_callback2(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
        event_queue_push((event) { .type = KEY_PRESSED, .data.key_pressed = { key } });
    else if (action == GLFW_RELEASE)
        event_queue_push((event) { .type = KEY_RELEASED, .data.key_released = { key } });
}
void mouse_button_callback2(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS)
        event_queue_push((event) { .type = MOUSE_BUTTON_PRESSED, .data.mouse_button_pressed = { button } });
    else if (action == GLFW_RELEASE)
        event_queue_push((event) { .type = MOUSE_BUTTON_RELEASED, .data.mouse_button_released = { button } });
}
void cursor_position_callback2(GLFWwindow* window, double xpos, double ypos)
{
    event_queue_push((event) { .type = MOUSE_MOVED, .data.mouse_moved = { xpos, ypos } });
}
void scroll_callback2(GLFWwindow* window, double xoffset, double yoffset)
{
    event_queue_push((event) { .type = MOUSE_SCROLLED, .data.mouse_scrolled = { xoffset, yoffset } });
}

//button callbacks
void startButton(void* currentStage)
{
    (*(int*)currentStage) = STAGE_IN_GAME;
}

void settingsButton(void* param)
{
    printf("settings\n");
}

void quitButton(void* currentStage)
{
    (*(int*)currentStage) = STAGE_QUIT;
}