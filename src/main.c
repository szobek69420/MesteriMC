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
#include "texture_handler/texture_handler.h"

#include "world/chunk/chunk.h"
#include "world/chunk/chunkManager.h"

#include "renderer/framebuffer/framebuffer.h"

#include "font_handler/font_handler.h"

#include "glm2/mat4.h"
#include "glm2/vec3.h"
#include "glm2/mat3.h"

#include "utils/list.h"

GLFWwindow* init_window(const char* name, int width, int height);
void handle_event(event e);

void init_renderer();
void end_renderer();
void render(camera* cum, font* f);

void init_kuba();
void end_kuba();
void draw_kuba(camera* cum, mat4* projection);
void update_kuba(camera* cum);

void render_text(font* f, const char* text, float x, float y, float scale);

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
    fontHandler_init();

    camera cum = camera_create(vec3_create2(0, 200, 0), vec3_create2(0, 1, 0), 0, 0, 90, 50, 0.2);

    init_kuba();
    init_renderer();

    textureHandler_importTextures();
    font f = fontHandler_loadFont("../assets/fonts/Monocraft.ttf", 48);

    float deltaTime;
    float lastFrame=glfwGetTime();
    float lastSecond = 0;//az fps szamolashoz
    int framesInLastSecond = 0;
    while (!glfwWindowShouldClose(window))
    {
        deltaTime = glfwGetTime() - lastFrame;
        lastFrame = glfwGetTime();
        lastSecond += deltaTime;
        framesInLastSecond++;
        if (lastSecond > 1)
        {
            printf("FPS: %d\n", framesInLastSecond);
            printf("Pos: %d %d %d\n\n", (int)cum.position.x, (int)cum.position.y, (int)cum.position.z);
            lastSecond = 0;
            framesInLastSecond = 0;
        }

        //update
        input_update();
        event e;
        while ((e = event_queue_poll()).type != NONE)
            handle_event(e);
        
        camera_update(&cum, deltaTime);

        update_kuba(&cum);

        //render
        render(&cum, &f);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    end_kuba();
    textureHandler_destroyTextures();
    fontHandler_close();
    end_renderer();

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
    glfwSwapInterval(0);

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

renderer rendor;
shader shadowShader;
shader geometryPassShader;
shader lightingPassShader;
shader forwardPassShader;
shader rectangleShader;
shader textShader;

unsigned int rectangleVAO;
unsigned int rectangleVBO;

unsigned int textVAO;
unsigned int textVBO;

void init_renderer()
{
    //rendor = renderer_create(window_getWidth(), window_getHeight());
    rendor = renderer_create(1920,1080);

    //shaders
    shadowShader = shader_import(
        "../assets/shaders/renderer/shadow/shader_shadow.vag",
        "../assets/shaders/renderer/shadow/shader_shadow.fag",
        NULL);

    geometryPassShader = shader_import(
        "../assets/shaders/renderer/deferred_geometry/shader_deferred_geometry.vag",
        "../assets/shaders/renderer/deferred_geometry/shader_deferred_geometry.fag",
        "../assets/shaders/renderer/deferred_geometry/shader_deferred_geometry.gag");

    glUseProgram(geometryPassShader.id);
    glUniform1i(glGetUniformLocation(geometryPassShader.id, "texture_albedo"), 0);
    glUniform1i(glGetUniformLocation(geometryPassShader.id, "texture_normal"), 1);
    glUniform1i(glGetUniformLocation(geometryPassShader.id, "texture_specular"), 2);

    lightingPassShader = shader_import(
        "../assets/shaders/renderer/deferred_lighting/shader_deferred_lighting.vag",
        "../assets/shaders/renderer/deferred_lighting/shader_deferred_lighting.fag",
        NULL
    );
    glUseProgram(lightingPassShader.id);
    glUniform1i(glGetUniformLocation(lightingPassShader.id, "texture_normal"), 0);
    glUniform1i(glGetUniformLocation(lightingPassShader.id, "texture_albedospec"), 1);
    glUniform1i(glGetUniformLocation(lightingPassShader.id, "texture_shadow"), 2);

    rectangleShader = shader_import(
        "../assets/shaders/renderer/rectangle/shader_rectangle.vag",
        "../assets/shaders/renderer/rectangle/shader_rectangle.fag",
        NULL
    );
    glUseProgram(rectangleShader.id);
    glUniform1i(glGetUniformLocation(rectangleShader.id, "tex"), 0);

    textShader = shader_import(
        "../assets/shaders/renderer2D/text/shader_text.vag",
        "../assets/shaders/renderer2D/text/shader_text.fag",
        NULL
    );

    glUseProgram(0);

    //rectangle VAO, VBO
    float vertices[] = {
        1,-1,0,     1,0,
        -1,-1,0,     0,0,
        1,1,0,      1,1,
        1,1,0,      1,1,
        -1,-1,0,    0,0,
        -1,1,0,     0,1
    };

    glGenVertexArrays(1, &rectangleVAO);
    glBindVertexArray(rectangleVAO);

    glGenBuffers(1, &rectangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER,rectangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    //text VAO, VBO
    glGenVertexArrays(1, &textVAO);
    glBindVertexArray(textVAO);

    glGenBuffers(1, &textVBO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glClearDepth(1);

    //cull front faces
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void end_renderer()
{
    renderer_destroy(rendor);

    shader_delete(&shadowShader);
    shader_delete(&geometryPassShader);
    shader_delete(&lightingPassShader);

    glDeleteVertexArrays(1, &rectangleVAO);
    glDeleteBuffers(1, &rectangleVBO);
}

void render(camera* cum, font* f)
{
    glEnable(GL_CULL_FACE);
    
    //shadow
    glViewport(0, 0, RENDERER_SHADOW_RESOLUTION, RENDERER_SHADOW_RESOLUTION);
    glBindFramebuffer(GL_FRAMEBUFFER, rendor.shadowBuffer.id);

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(shadowShader.id);
    //set up lightmatrix
    //render geometry


    //geometry pass
    glViewport(0, 0, 1920, 1080);
    //glViewport(0, 0, window_getWidth(), window_getHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, rendor.gBuffer.id);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    mat4 projection = mat4_perspective(cum->fov, window_getAspect(), 0.1, 300);
    mat4 pv= mat4_multiply(projection, camera_getViewMatrix(cum));

    glUseProgram(geometryPassShader.id);

    glUniformMatrix4fv(glGetUniformLocation(geometryPassShader.id, "pv"), 1, GL_FALSE, pv.data);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_ATLAS_ALBEDO));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_ATLAS_NORMAL));

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_ATLAS_SPECULAR));

    draw_kuba(cum, &projection);

    //lighting pass
    glDisable(GL_DEPTH_TEST);
    
    glBindFramebuffer(GL_FRAMEBUFFER, rendor.endBuffer.id);

    glClearColor(0.0666f, 0.843f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rendor.gBuffer.normal);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, rendor.gBuffer.albedoSpec);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rendor.shadowBuffer.depthBuffer);

    glUseProgram(lightingPassShader.id);

    glBindVertexArray(rectangleVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    //draw deferred results to screen
    glViewport(0, 0, window_getWidth(), window_getHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);//default framebuffer
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rendor.endBuffer.colorBuffer);

    glUseProgram(rectangleShader.id);
    glUniformMatrix4fv(glGetUniformLocation(geometryPassShader.id, "projectionToWorld"), 1, GL_FALSE, mat4_inverse(pv).data);
    glBindVertexArray(rectangleVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    //copy depth buffer
    /*glBindFramebuffer(GL_READ_FRAMEBUFFER, rendor.gBuffer.id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, 1920, 1080, 0, 0, window_getWidth(), window_getHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

    //render forward scene


    //render 2d stuff (only text yet)
    //glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glUseProgram(textShader.id);
    
    mat4 projection2D = mat4_ortho(0, window_getWidth(), 0, window_getHeight(), -1, 1);
    glUniformMatrix4fv(glGetUniformLocation(textShader.id, "projection"), 1, GL_FALSE, projection2D.data);
    glUniform3f(glGetUniformLocation(textShader.id, "textColor"), 1, 1, 1);
    
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);
    render_text(f, "amogus", 10, 10, 1);
    //glDisable(GL_BLEND);
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
chunkManager cm;
void init_kuba()
{
    program = shader_import("../assets/shaders/chunk/chunkTest.vag", "../assets/shaders/chunk/chunkTest.fag", NULL);
    shader_use(program.id);
    shader_setInt(program.id, "tex", 0);
    shader_use(0);
    cm = chunkManager_create(69, 4);
}

void end_kuba() {
    shader_delete(&program);
    chunkManager_destroy(&cm);
}

void draw_kuba(camera* cum, mat4* projection) {
    chunkManager_drawTerrain(&cm, &geometryPassShader, cum, projection);
}

void update_kuba(camera* cum)
{
    int chunkX, chunkY, chunkZ;
    chunk_getChunkFromPos(vec3_create2(cum->position.x+CHUNK_WIDTH*0.5f, cum->position.y + CHUNK_HEIGHT * 0.5f, cum->position.z + CHUNK_WIDTH * 0.5f), &chunkX, &chunkY, &chunkZ);

    chunkManager_searchForUpdates(&cm, chunkX, chunkY, chunkZ);
    chunkManager_update(&cm);
    chunkManager_update(&cm);
}

void render_text(font* f, const char* text, float x, float y, float scale)
{
    // assuming textShader is used, uniforms are set and VAO is bound
    // iterate through all characters
    for (int i = 0; text[i] != '\0'; i++) 
    {
        character ch = f->characters[text[i]];

        float xpos = x + ch.bearingX * scale;
        float ypos = y - (ch.height - ch.bearingY) * scale;

        float w = ch.width * scale;
        float h = ch.height * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}