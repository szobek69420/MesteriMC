#define _CRT_SECURE_NO_WARNINGS

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "window/window.h"
#include "camera/camera.h"
#include "event/event_queue.h"
#include "input/input.h"
#include "shader/shader.h"

#include "world/chunk/chunk.h"

#include "glm2/mat4.h"
#include "glm2/vec3.h"
#include "glm2/mat3.h"

GLFWwindow* init_window(const char* name, int width, int height);
void handle_event(event e);
void render();

void init_kuba();
void end_kuba();
void draw_kuba(camera* cum);

//glfw callbacks
void window_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main()
{
    window_setWidth(1300);
    window_setHeight(800);
    GLFWwindow* window = init_window("amogus", window_getWidth(), window_getHeight());
    event_queue_init();
    input_init();

    camera cum = camera_create(vec3_create2(0, 0, 0), vec3_create2(0, 1, 0), 0, 0, 90, 20, 0.8);

    shader shader = shader_import("../assets/shaders/amoma.vag", "../assets/shaders/amoma.fag", NULL);
    shader_delete(&shader);

    init_kuba();

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glClearDepth(1);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    while (!glfwWindowShouldClose(window))
    {
        //update
        input_update();
        event e;
        while ((e = event_queue_poll()).type != NONE)
            handle_event(e);
        
        camera_update(&cum, 0.001f);


        //render
        render();
        draw_kuba(&cum);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    end_kuba();
    glfwTerminate();
    return 69;
}

GLFWwindow* init_window(const char* name, int width, int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, name, NULL, NULL);
    if (window == NULL)//check if glfw is kaputt
    {
        printf("Failed to create GLFW window");
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);

    //callbacks
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //check if glad is kaputt
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD");
        return NULL;
    }

    glViewport(0, 0, width, height);

    return window;
}
void handle_event(event e)
{
    switch (e.type)
    {
    case WINDOW_RESIZE:
        window_setWidth(e.data.window_resize.width);
        window_setHeight(e.data.window_resize.height);
        glViewport(0, 0, e.data.window_resize.width, e.data.window_resize.height);
        break;
    default:
        input_handle_event(e);
        if (e.type == KEY_PRESSED && e.data.key_pressed.key_code == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
        break;
    }
}
void render()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    if(event_queue_back().type!=WINDOW_RESIZE)
        event_queue_push((event){ .type = WINDOW_RESIZE, .data.window_resize = { width, height } });
    else
        event_queue_swap_back((event) { .type = WINDOW_RESIZE, .data.window_resize = { width, height } });
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
        event_queue_push((event){ .type = KEY_PRESSED, .data.key_pressed = { key } });
    else if (action == GLFW_RELEASE)
        event_queue_push((event){ .type = KEY_RELEASED, .data.key_released = { key } });
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS)
        event_queue_push((event){ .type = MOUSE_BUTTON_PRESSED, .data.mouse_button_pressed = { button } });
    else if (action == GLFW_RELEASE)
        event_queue_push((event){ .type = MOUSE_BUTTON_RELEASED, .data.mouse_button_released = { button } });
}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    event_queue_push((event){ .type = MOUSE_MOVED, .data.mouse_moved = { xpos, ypos } });
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    event_queue_push((event){ .type = MOUSE_SCROLLED, .data.mouse_scrolled = { xoffset, yoffset } });
}

shader program;
chunk kuba;
void init_kuba()
{
    program = shader_import("../assets/shaders/chunk/chunkTest.vag", "../assets/shaders/chunk/chunkTest.fag", NULL);
    kuba = chunk_generate(0, 0, 0);
}

void end_kuba() {
    shader_delete(&program);
    chunk_destroy(&kuba);
}

void draw_kuba(camera* cum) {
    glUseProgram(program.id);

    mat4 model = mat4_create(1);
    //model = mat4_rotate(model, vec3_create2(0, 0.4, 1), 50*glfwGetTime());
    //model = mat4_rotate(model, vec3_create2(-3, -2, 1), 70 * glfwGetTime());
    //model = mat4_translate(model, vec3_create2(0, 0, -0.5f));


    shader_setMat4(program.id, "model", model);
    shader_setMat4(program.id, "view", camera_get_view_matrix(cum));
    shader_setMat4(program.id, "projection", mat4_perspective(cum->fov, window_getAspect(), 0.1, 200));
    //shader_setMat4(program.id, "projection", mat4_ortho(-50,50,-50,50,1,100));

    
    chunk_drawTerrain(&kuba);
}