#define _CRT_SECURE_NO_WARNINGS

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "event/event_queue.h"
#include "input/input.h"
#include "shader/shader.h"

#include "glm2/mat4.h"
#include "glm2/vec3.h"

GLFWwindow* init_window(const char* name, int width, int height);
void handle_event(event e);
void render();

void init_kuba();
void draw_kuba();

//glfw callbacks
void window_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main()
{
    GLFWwindow* window = init_window("amogus", 600, 600);
    event_queue_init();
    input_init();

    shader shader = shader_import("../assets/shaders/amoma.vag", "../assets/shaders/amoma.fag", NULL);
    shader_delete(&shader);

    init_kuba();

    while (!glfwWindowShouldClose(window))
    {
        //update
        input_update();
        event e;
        while ((e = event_queue_poll()).type != NONE)
            handle_event(e);

        //render
        render();

        draw_kuba();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

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
        glViewport(0, 0, e.data.window_resize.width, e.data.window_resize.height);
        break;
    default:
        input_handle_event(e);
        break;
    }
}
void render()
{
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    event_queue_push((event){ .type = WINDOW_RESIZE, .data.window_resize = { width, height } });
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
unsigned int vao, vbo;
void init_kuba()
{
    float vertices[] = {
        1,-1,1,     1,0,0,
        -1,-1,1,    1,0,0,
        0,1,0,      1,0,0,

        1,-1,-1,     0.8f,0,0,
        1,-1,1,    0.8f,0,0,
        0,1,0,      0.8f,0,0,

        -1,-1,-1,    0.4f,0,0,
        1,-1,-1,   0.4f,0,0,
        0,1,0,      0.4f,0,0,

        -1,-1,1,   0.6f,0,0,
        -1,-1,-1,    0.6f,0,0,
        0,1,0,      0.6f,0,0,

        -1,-1,1,    0.2f,0,0,
        1,-1,1,     0.2f,0,0,
        1,-1,-1,    0.2f,0,0,

        1,-1,-1,    0.2f,0,0,
        -1,-1,-1,   0.2f,0,0,
        -1,-1,1,    0.2f,0,0
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    program = shader_import("../assets/shaders/amoma.vag", "../assets/shaders/amoma.fag", NULL);
}

void draw_kuba() {
    glUseProgram(program.id);

    mat4 model = mat4_create(1);
    model = mat4_rotate(model, vec3_create2(0, 0.4, 1), 50*glfwGetTime());
    model = mat4_rotate(model, vec3_create2(-3, -2, 1), 70 * glfwGetTime());
    //model = mat4_translate(model, vec3_create2(0, 0, -0.5f));

    shader_setMat4(program.id, "model", model);
    shader_setMat4(program.id, "view", mat4_lookAt(vec3_create2(-6, 5, 10), vec3_create2(6, -5, -10), vec3_create2(0, 1, 0)));
    shader_setMat4(program.id, "projection", mat4_perspective(40, 1, 0.1, 30));

    //shader_setMat4(program.id, "view", mat4_lookAt(vec3_create2(-8, 10, 10), vec3_create2(0.8, -1,-1), vec3_create2(0, 1, 0)));
    //shader_setMat4(program.id, "projection", mat4_create(1));

    
    glBindVertexArray(vao);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 18);
}