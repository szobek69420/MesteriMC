#define _CRT_SECURE_NO_WARNINGS
#include "main_menu.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../stages.h"

#include "../../texture_handler/texture_handler.h"

#include "../../window/window.h"

#include "../../settings/settings.h"

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

enum {
    MENU_MAIN, MENU_SETTINGS
};
int menuState;

font f;

canvas* vaszonBackground;
int background;

canvas* vaszon;

canvas* vaszonSettings;
int sliderRenderDistance, textRenderDistance;
int sliderResolution, textResolution;
int buttonShadows;
int sliderShadowResolution, textShadowResolution;

//glfw callbacks
void window_size_callback2(GLFWwindow* window, int width, int height);
void key_callback2(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback2(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback2(GLFWwindow* window, double xpos, double ypos);
void scroll_callback2(GLFWwindow* window, double xoffset, double yoffset);

//button callbacks
void startButton(int inBounds, void* currentStage);
void settingsButton(int inBounds, void* destination);
void quitButton(int inBounds, void* currentStage);
void sliderRenderDistanceFunction(float value);
void sliderResolutionFunction(float value);
void buttonShadowFunction(int inBounds, void* param);
void sliderShadowResolutionFunction(float value);

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
    menuState = MENU_MAIN;
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

        switch (menuState)
        {
        case MENU_MAIN:
            canvas_checkMouseInput(vaszon, x, y, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT));
            canvas_render(vaszonBackground, 0, 0, 0);
            canvas_render(vaszon, x, y, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT));
            break;

        case MENU_SETTINGS:
            canvas_checkMouseInput(vaszonSettings, x, y, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT));
            canvas_render(vaszonBackground, 0, 0, 0);
            canvas_render(vaszonSettings, x, y, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT));
            break;
        }

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

    int temp;
    char buffer[50];
    //background canvas
    vaszonBackground = canvas_create(window_getWidth(), window_getHeight(), NULL);
    background = canvas_addImage(vaszonBackground, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, 0, 0, 0, textureHandler_getTexture(TEXTURE_MENU_BACKGROUND));
    background_resize();

    //main canvas
    vaszon = canvas_create(window_getWidth(), window_getHeight(), "../assets/fonts/Monocraft.ttf");

    canvas_addImage(vaszon, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_TOP, 0, 100, 700, 100, textureHandler_getTexture(TEXTURE_MENU_TITLE));

    temp = canvas_addButton(vaszon, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, 0, 400, 60);
    canvas_setButtonText(vaszon, temp, "mom's spaghetti", 24, CANVAS_COLOUR_ACCENT_0);
    canvas_setButtonBorder(vaszon, temp, 5, 5);
    canvas_setButtonFillColour(vaszon, temp, CANVAS_COLOUR_PRIMARY_1);
    canvas_setButtonBorderColour(vaszon, temp, CANVAS_COLOUR_PRIMARY_0);
    canvas_setButtonClicked(vaszon, temp, startButton, currentStage);

    temp = canvas_addButton(vaszon, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, -70, 400, 60);
    canvas_setButtonText(vaszon, temp, "he keeps on adjustin'", 24, CANVAS_COLOUR_ACCENT_0);
    canvas_setButtonBorder(vaszon, temp, 5, 5);
    canvas_setButtonFillColour(vaszon, temp, CANVAS_COLOUR_PRIMARY_1);
    canvas_setButtonBorderColour(vaszon, temp, CANVAS_COLOUR_PRIMARY_0);
    canvas_setButtonClicked(vaszon, temp, settingsButton, MENU_SETTINGS);

    temp = canvas_addButton(vaszon, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, -140, 400, 60);
    canvas_setButtonText(vaszon, temp, "step back to reality", 24, CANVAS_COLOUR_ACCENT_0);
    canvas_setButtonBorder(vaszon, temp, 5, 5);
    canvas_setButtonFillColour(vaszon, temp, CANVAS_COLOUR_PRIMARY_1);
    canvas_setButtonBorderColour(vaszon, temp, CANVAS_COLOUR_PRIMARY_0);
    canvas_setButtonClicked(vaszon, temp, quitButton, currentStage);

    canvas_addText(vaszon, "made by:", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_BOTTOM, 10, 40, CANVAS_COLOUR_ACCENT_0, 16);
    canvas_addText(vaszon, "Szobek Adam Mark", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_BOTTOM, 10, 25, 0, 1, 1, 16);
    canvas_addText(vaszon, "Mandli Ors", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_BOTTOM, 10, 10, 0, 1, 1, 16);

    //settings canvas
    vaszonSettings = canvas_create(window_getWidth(), window_getHeight(), "../assets/fonts/Monocraft.ttf");

    canvas_addImage(vaszonSettings, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_TOP, 0, 100, 550, 100, textureHandler_getTexture(TEXTURE_MENU_TITLE_SETTINGS));

    temp = canvas_addButton(vaszonSettings, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_BOTTOM, 0, 100, 300, 60);
    canvas_setButtonText(vaszonSettings, temp, "return to main", 24, CANVAS_COLOUR_ACCENT_0);
    canvas_setButtonBorder(vaszonSettings, temp, 5, 5);
    canvas_setButtonFillColour(vaszonSettings, temp, CANVAS_COLOUR_PRIMARY_1);
    canvas_setButtonBorderColour(vaszonSettings, temp, CANVAS_COLOUR_PRIMARY_0);
    canvas_setButtonClicked(vaszonSettings, temp, settingsButton, MENU_MAIN);


    sliderRenderDistance = canvas_addSlider(vaszonSettings, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, -220, 40, 400, 50, 10, 50, 69);
    canvas_setSliderBounds(vaszonSettings, sliderRenderDistance, 2, 16);
    canvas_setSliderValue(vaszonSettings, sliderRenderDistance, settings_getInt(SETTINGS_RENDER_DISTANCE));
    canvas_setSliderBackgroundBorder(vaszonSettings, sliderRenderDistance, 5, 5);
    canvas_setSliderBackgroundBorderColour(vaszonSettings, sliderRenderDistance, CANVAS_COLOUR_PRIMARY_0);
    canvas_setSliderBackgroundFillColour(vaszonSettings, sliderRenderDistance, CANVAS_COLOUR_PRIMARY_1);
    canvas_setSliderKnobFillColour(vaszonSettings, sliderRenderDistance, 0,1,1);
    canvas_setSliderKnobBorder(vaszonSettings, sliderRenderDistance, 0, 0);
    canvas_setSliderCallback(vaszonSettings, sliderRenderDistance, sliderRenderDistanceFunction);
    
    sprintf(buffer, "render distance: %d", settings_getInt(SETTINGS_RENDER_DISTANCE));
    textRenderDistance = canvas_addText(vaszonSettings, buffer, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, -220, 40, CANVAS_COLOUR_ACCENT_0, 24);


    sliderResolution = canvas_addSlider(vaszonSettings, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 220, 40, 400, 50, 10, 50, 69);
    canvas_setSliderBounds(vaszonSettings, sliderResolution, 0, 3);
    canvas_setSliderValue(vaszonSettings, sliderResolution, settings_getInt(SETTINGS_RENDERER_RESOLUTION));
    canvas_setSliderBackgroundBorder(vaszonSettings, sliderResolution, 5, 5);
    canvas_setSliderBackgroundBorderColour(vaszonSettings, sliderResolution, CANVAS_COLOUR_PRIMARY_0);
    canvas_setSliderBackgroundFillColour(vaszonSettings, sliderResolution, CANVAS_COLOUR_PRIMARY_1);
    canvas_setSliderKnobFillColour(vaszonSettings, sliderResolution, 0, 1, 1);
    canvas_setSliderKnobBorder(vaszonSettings, sliderResolution, 0, 0);
    canvas_setSliderCallback(vaszonSettings, sliderResolution, sliderResolutionFunction);

    textResolution = canvas_addText(vaszonSettings, "amogus", CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 220, 40, CANVAS_COLOUR_ACCENT_0, 24);
    sliderResolutionFunction(settings_getInt(SETTINGS_RENDERER_RESOLUTION));


    buttonShadows = canvas_addButton(vaszonSettings, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, -220, -40, 400, 50);
    if (settings_getInt(SETTINGS_SHADOWS) == 0) canvas_setButtonText(vaszonSettings, buttonShadows, "shadows: off", 24, CANVAS_COLOUR_ACCENT_0);
    else canvas_setButtonText(vaszonSettings, buttonShadows, "shadows: on", 24, CANVAS_COLOUR_ACCENT_0);
    canvas_setButtonBorder(vaszonSettings, buttonShadows, 5, 5);
    canvas_setButtonFillColour(vaszonSettings, buttonShadows, CANVAS_COLOUR_PRIMARY_1);
    canvas_setButtonBorderColour(vaszonSettings, buttonShadows, CANVAS_COLOUR_PRIMARY_0);
    canvas_setButtonClicked(vaszonSettings, buttonShadows, buttonShadowFunction, NULL);


    sliderShadowResolution = canvas_addSlider(vaszonSettings, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 220, -40, 400, 50, 10, 50, 69);
    canvas_setSliderBounds(vaszonSettings, sliderShadowResolution, 0, 3);
    canvas_setSliderValue(vaszonSettings, sliderShadowResolution, settings_getInt(SETTINGS_SHADOW_RESOLUTION));
    canvas_setSliderBackgroundBorder(vaszonSettings, sliderShadowResolution, 5, 5);
    canvas_setSliderBackgroundBorderColour(vaszonSettings, sliderShadowResolution, CANVAS_COLOUR_PRIMARY_0);
    canvas_setSliderBackgroundFillColour(vaszonSettings, sliderShadowResolution, CANVAS_COLOUR_PRIMARY_1);
    canvas_setSliderKnobFillColour(vaszonSettings, sliderShadowResolution, 0,1,1);
    canvas_setSliderKnobBorder(vaszonSettings, sliderShadowResolution, 0, 0);
    canvas_setSliderCallback(vaszonSettings, sliderShadowResolution, sliderShadowResolutionFunction);

    textShadowResolution = canvas_addText(vaszonSettings, "amogus", CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 220, -40, CANVAS_COLOUR_ACCENT_0, 24);
    sliderShadowResolutionFunction(settings_getInt(SETTINGS_SHADOW_RESOLUTION));
}

void end()
{
    fontHandler_close();
    fontHandler_destroyFont(&f);
    canvas_destroy(vaszonBackground);
    canvas_destroy(vaszon);
    canvas_destroy(vaszonSettings);
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
        canvas_setComponentSize(vaszonBackground, background, window_getHeight() * bgAspectXY, window_getHeight());
    }
    else
    {
        canvas_setComponentSize(vaszonBackground, background, window_getWidth(), window_getWidth() / bgAspectXY);
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
        canvas_setSize(vaszonBackground, window_getWidth(), window_getHeight());
        canvas_setSize(vaszon, window_getWidth(), window_getHeight());
        canvas_setSize(vaszonSettings, window_getWidth(), window_getHeight());
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
void startButton(int inBounds, void* currentStage)
{
    if (inBounds == 0)
        return;
    (*(int*)currentStage) = STAGE_IN_GAME;
}

void settingsButton(int inBounds, void* destination)
{
    if (inBounds == 0)
        return;
    switch ((int)destination)
    {
    case MENU_MAIN:
        settings_save();
        menuState = MENU_MAIN;
        break;

    case MENU_SETTINGS:
        menuState = MENU_SETTINGS;
        break;
    }
}

void quitButton(int inBounds, void* currentStage)
{
    if (inBounds == 0)
        return;
    (*(int*)currentStage) = STAGE_QUIT;
}

void sliderRenderDistanceFunction(float value)
{
    settings_setInt(SETTINGS_RENDER_DISTANCE, lroundf(value));

    static char buffer[30];
    sprintf(buffer, "render distance: %d", lroundf(value));
    canvas_setTextText(vaszonSettings, textRenderDistance, buffer);
}

void sliderResolutionFunction(float value)
{
    settings_setInt(SETTINGS_RENDERER_RESOLUTION, lroundf(value));

    static char buffer[30];
    sprintf(buffer, "renderer res.: %dx%d", settings_getInt(SETTINGS_RENDERER_WIDTH), settings_getInt(SETTINGS_RENDERER_HEIGHT));
    canvas_setTextText(vaszonSettings, textResolution, buffer);
}

void buttonShadowFunction(int inBounds, void* param)
{
    if (inBounds == 0)
        return;

    if (settings_getInt(SETTINGS_SHADOWS) == 0)
    {
        settings_setInt(SETTINGS_SHADOWS, 1);
        canvas_setButtonText(vaszonSettings, buttonShadows, "shadows: on", 24, 1, 0.85f, 0);
    }
    else
    {
        settings_setInt(SETTINGS_SHADOWS, 0);
        canvas_setButtonText(vaszonSettings, buttonShadows,  "shadows: off", 24, 1, 0.85f, 0);
    }
}

void sliderShadowResolutionFunction(float value)
{
    settings_setInt(SETTINGS_SHADOW_RESOLUTION, lroundf(value));

    static char buffer[30];
    sprintf(buffer, "shadow res.: %dx%d", settings_getInt(SETTINGS_SHADOW_RESOLUTION_PIXELS), settings_getInt(SETTINGS_SHADOW_RESOLUTION_PIXELS));
    canvas_setTextText(vaszonSettings, textShadowResolution, buffer);
}