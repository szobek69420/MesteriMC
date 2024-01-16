#define _CRT_SECURE_NO_WARNINGS

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "window/window.h"
#include "camera/camera.h"
#include "event/event_queue.h"
#include "input/input.h"
#include "shader/shader.h"
#include "texture_handler/texture_handler.h"

#include "world/chunk/chunk.h"
#include "world/chunk/chunkManager.h"
#include "world/sun/sun.h"

#include "renderer/framebuffer/framebuffer.h"
#include "renderer/light/light.h"
#include "mesh/sphere/sphere.h"
#include "mesh/kuba/kuba.h"

#include "font_handler/font_handler.h"

#include "glm2/mat4.h"
#include "glm2/vec3.h"
#include "glm2/mat3.h"

#include "utils/list.h"
#include "utils/vector.h"

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
void render_cube();

double lerp(double a, double b, double f);

//glfw callbacks
void window_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main()
{
    printf("%.2f\n", 57.295779513f * acosf(0.88f));
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
            //printf("FPS: %d\n", framesInLastSecond);
            //printf("Pos: %d %d %d\n\n", (int)cum.position.x, (int)cum.position.y, (int)cum.position.z);
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
        static char buffer[50];
        sprintf(buffer, "FPS: %.0f", 1.0 / deltaTime);
        render_text(&f, buffer, 15, window_getHeight() - 34, 0.5);
        sprintf(buffer, "Pos: %d %d %d", (int)cum.position.x, (int)cum.position.y, (int)cum.position.z);
        render_text(&f, buffer, 15, window_getHeight() - 69, 0.5);

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

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
shader ssaoShader;
shader ssaoBlurShader;
shader lightingPassShader;
shader forwardPassShader;
shader finalPassShader;
shader textShader;
shader skyboxShader; mesh skyboxMesh;

unsigned int rectangleVAO;
unsigned int rectangleVBO;

unsigned int textVAO;
unsigned int textVBO;

vector* ssaoKernel;
vector* ssaoNoise;
unsigned int noiseTexture;

vector* lights;
light sunTzu;

light_renderer lightRenderer;

sun szunce;

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

    ssaoShader = shader_import(
        "../assets/shaders/renderer/ssao/shader_ssao.vag",
        "../assets/shaders/renderer/ssao/shader_ssao.fag",
        NULL
    );
    glUseProgram(ssaoShader.id);
    glUniform1i(glGetUniformLocation(ssaoShader.id, "texture_normal"), 0);
    glUniform1i(glGetUniformLocation(ssaoShader.id, "texture_depth"), 1);
    glUniform1i(glGetUniformLocation(ssaoShader.id, "texture_noise"), 2);
    glUniform1f(glGetUniformLocation(ssaoShader.id, "onePerScreenWidth"), 1.0f / RENDERER_WIDTH);
    glUniform1f(glGetUniformLocation(ssaoShader.id, "onePerScreenHeight"), 1.0f / RENDERER_HEIGHT);

    ssaoBlurShader = shader_import(
        "../assets/shaders/renderer/ssao/shader_ssao.vag",
        "../assets/shaders/renderer/ssao/shader_ssao_blur.fag",
        NULL
    );
    glUseProgram(ssaoBlurShader.id);
    glUniform1i(glGetUniformLocation(ssaoBlurShader.id, "ssaoInput"), 0);

    lightingPassShader = shader_import(
        "../assets/shaders/renderer/deferred_lighting/shader_deferred_lighting.vag",
        "../assets/shaders/renderer/deferred_lighting/shader_deferred_lighting.fag",
        NULL
    );
    glUseProgram(lightingPassShader.id);
    glUniform1i(glGetUniformLocation(lightingPassShader.id, "texture_normal"), 0);
    glUniform1i(glGetUniformLocation(lightingPassShader.id, "texture_albedospec"), 1);
    glUniform1i(glGetUniformLocation(lightingPassShader.id, "texture_shadow"), 2);
    glUniform1i(glGetUniformLocation(lightingPassShader.id, "texture_depth"), 3);
    glUniform1i(glGetUniformLocation(lightingPassShader.id, "texture_ssao"), 4);
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "onePerScreenWidth"), 1.0f / RENDERER_WIDTH);
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "onePerScreenHeight"), 1.0f / RENDERER_HEIGHT);
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogStart"), 100);
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogEnd"), 120);
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogHelper"), 1.0f/(120-100));

    forwardPassShader = shader_import(
        "../assets/shaders/renderer/forward/shader_forward.vag",
        "../assets/shaders/renderer/forward/shader_forward.fag",
        NULL
    );

    finalPassShader = shader_import(
        "../assets/shaders/renderer/final_pass/shader_final_pass.vag",
        "../assets/shaders/renderer/final_pass/shader_final_pass.fag",
        NULL
    );
    glUseProgram(finalPassShader.id);
    glUniform1i(glGetUniformLocation(finalPassShader.id, "tex"), 0);
    glUniform1f(glGetUniformLocation(finalPassShader.id, "exposure"), 0.3);

    textShader = shader_import(
        "../assets/shaders/renderer2D/text/shader_text.vag",
        "../assets/shaders/renderer2D/text/shader_text.fag",
        NULL
    );



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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //ssao shit
    // generate sample kernel
    // ----------------------
    ssaoKernel = vector_create(64);
    for (unsigned int i = 0; i < 64; ++i)
    {
        vec3* sample = (vec3*)malloc(sizeof(vec3));
        *sample = vec3_create2(
            (rand() % 1001) / 1000.0 * 2.0 - 1.0, //random between [-1.0 - 1.0]
            (rand() % 1001) / 1000.0 * 2.0 - 1.0, //random between [-1.0 - 1.0]
            (rand() % 1001) / 1000.0              //random between [ 0.0 - 1.0]
        );
        // scale samples s.t. they're more aligned to center of kernel
        *sample = vec3_normalize(*sample);
        double scale = i / 64.0;
        scale = lerp(0.1, 1.0, scale * scale);
        *sample = vec3_scale(*sample, (rand() % 1001) / 1000.0 * scale); //random between [0.0 - 1.0] * scale
        vector_push_back(ssaoKernel, sample);
    }
    // generate noise texture
    // ----------------------
    ssaoNoise = vector_create(16);
    for (unsigned int i = 0; i < 16; i++)
    {
        vec3* noise = (vec3*)malloc(sizeof(vec3));
        *noise = vec3_create2(
            (rand() % 1001) / 1000.0 * 2.0 - 1.0, //random between [-1.0 - 1.0]
            (rand() % 1001) / 1000.0 * 2.0 - 1.0, //random between [-1.0 - 1.0]
            0.0f
        ); // rotate around z-axis (in tangent space)
        vector_push_back(ssaoNoise, noise);
    }
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, vector_get(ssaoNoise, 0));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    //light shit
    lightRenderer = light_createRenderer();
    srand(13);
    lights = vector_create(32);
    for (unsigned int i = 0; i < 32; i++)
    {
        light* lit = (light*)malloc(sizeof(light));
        *lit = light_create(
            vec3_create2((((rand() % 100) / 200.0f) + 0.5), (((rand() % 100) / 200.0f) + 0.5), (((rand() % 100) / 200.0f) + 0.5)),
            vec3_create2((((rand() % 100) / 100.0) * 50.0 - 3.0), 25 + (((rand() % 100) / 100.0) * 20.0 - 4.0), (((rand() % 100) / 100.0) * 100.0 - 3.0)),
            vec3_create2(10, 0.05, 0.05)
            );
        vector_push_back(lights, lit);
    }

    sunTzu = light_create(
        vec3_create2(1, 0.97, 0.4),
        vec3_create2(0.6, 1.3, 0.8),
        vec3_create2(3, 0, 0)
    );

    //skybox
    skyboxShader = shader_import(
        "../assets/shaders/skybox/shader_skybox.vag",
        "../assets/shaders/skybox/shader_skybox.fag",
        NULL
    );
    glUseProgram(skyboxShader.id);
    glUniform1i(glGetUniformLocation(skyboxShader.id, "skybox"), 0);
    glUseProgram(0);

    skyboxMesh = kuba_create();

    //sun
    szunce = sun_create();
}

void end_renderer()
{
    renderer_destroy(rendor);

    shader_delete(&shadowShader);
    shader_delete(&geometryPassShader);
    shader_delete(&lightingPassShader);

    glDeleteVertexArrays(1, &rectangleVAO);
    glDeleteBuffers(1, &rectangleVBO);

    //ssao cleanup
    for (int i = 0; i < ssaoKernel->size; i++)
        free((vec3*)vector_get(ssaoKernel, i));
    vector_destroy(ssaoKernel);
    for (int i = 0; i < ssaoNoise->size; i++)
        free((vec3*)vector_get(ssaoNoise, i));
    vector_destroy(ssaoNoise);

    //light cleanup
    for (int i = 0; i < lights->size; i++)
        free((light*)vector_get(lights, i));
    vector_destroy(lights);

    light_destroyRenderer(lightRenderer);

    //skybox
    shader_delete(&skyboxShader);
    mesh_destroy(skyboxMesh);

    //sun
    sun_destroy(&szunce);
}

void render(camera* cum, font* f)
{
    //shadow
    glViewport(0, 0, RENDERER_SHADOW_RESOLUTION, RENDERER_SHADOW_RESOLUTION);
    glBindFramebuffer(GL_FRAMEBUFFER, rendor.shadowBuffer.id);

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(shadowShader.id);


    //matrices
    mat4 view = camera_getViewMatrix(cum);
    mat4 projection = mat4_perspective(cum->fov, window_getAspect(), 0.1, 200);
    mat4 pv = mat4_multiply(projection, view);
    mat4 projectionInverse = mat4_inverse(projection);
    mat3 viewNormal = mat3_createFromMat(view);

    //prepare gbuffer fbo
    glViewport(0, 0, 1920, 1080);
    glBindFramebuffer(GL_FRAMEBUFFER, rendor.gBuffer.id);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //geometry pass ------------------------------------------------------------------------------------------
    glUseProgram(geometryPassShader.id);

    glUniformMatrix4fv(glGetUniformLocation(geometryPassShader.id, "view"), 1, GL_FALSE, view.data);
    glUniformMatrix4fv(glGetUniformLocation(geometryPassShader.id, "projection"), 1, GL_FALSE, projection.data);
    glUniformMatrix3fv(glGetUniformLocation(geometryPassShader.id, "view_normal"), 1, GL_FALSE, viewNormal.data);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_ATLAS_ALBEDO));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_ATLAS_NORMAL));

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_ATLAS_SPECULAR));

    draw_kuba(cum, &projection);

    //copy depth buffer
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, rendor.gBuffer.id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rendor.endBuffer.id);
    glBlitFramebuffer(0, 0, RENDERER_WIDTH, RENDERER_HEIGHT, 0, 0, RENDERER_WIDTH, RENDERER_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    //ssao pass ------------------------------------------------------------------------------------------
    // generate SSAO texture
    glBindFramebuffer(GL_FRAMEBUFFER, rendor.ssaoBuffer.idColor);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(ssaoShader.id);
        // Send kernel + rotation
        for (unsigned int i = 0; i < 64; ++i)
        {
            static char buffer[20];
            sprintf(buffer, "samples[%d]", i);
            glUniform3fv(glGetUniformLocation(ssaoShader.id, buffer), 1, (float*)vector_get(ssaoKernel, i));
        }
        glUniformMatrix4fv(glGetUniformLocation(ssaoShader.id, "projection"), 1, GL_FALSE, projection.data);
        glUniformMatrix4fv(glGetUniformLocation(ssaoShader.id, "projection_inverse"), 1, GL_FALSE, projectionInverse.data);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rendor.gBuffer.normal);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, rendor.gBuffer.depthBuffer);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glBindVertexArray(rectangleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // blur SSAO texture to remove noise
    glBindFramebuffer(GL_FRAMEBUFFER, rendor.ssaoBuffer.idBlur);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(ssaoBlurShader.id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rendor.ssaoBuffer.colorBuffer);
        glBindVertexArray(rectangleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //lighting pass ------------------------------------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, rendor.endBuffer.id);
    //glClearColor(0.0666f, 0.843f, 1.0f, 1.0f);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_GREATER);
    glFrontFace(GL_CW);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rendor.gBuffer.normal);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, rendor.gBuffer.albedoSpec);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rendor.shadowBuffer.depthBuffer);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, rendor.gBuffer.depthBuffer);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, rendor.ssaoBuffer.colorBufferBlur);

    glUseProgram(lightingPassShader.id);
    glUniformMatrix4fv(glGetUniformLocation(lightingPassShader.id, "view"), 1, GL_FALSE, view.data);
    glUniformMatrix4fv(glGetUniformLocation(lightingPassShader.id, "projection"), 1, GL_FALSE, projection.data);
    glUniformMatrix4fv(glGetUniformLocation(lightingPassShader.id, "projection_inverse"), 1, GL_FALSE, projectionInverse.data);

    //point lights
    light_setPosition((light*)vector_get(lights, 0), cum->position);
    float* bufferData = (float*)malloc(lights->size * LIGHT_SIZE_IN_VBO);
    light* tempLight;
    for (unsigned int i = 0; i < lights->size; i++)
    {
        tempLight = (light*)vector_get(lights, i);
        memcpy(bufferData + i * LIGHT_FLOATS_IN_VBO, &(tempLight->position.x), LIGHT_SIZE_IN_VBO);
    }

    light_fillRenderer(&lightRenderer, bufferData, lights->size);
    light_render(&lightRenderer, 69);
    free(bufferData);

    //directional
    bufferData = (float*)malloc(LIGHT_SIZE_IN_VBO);
    memcpy(bufferData, &(sunTzu.position.x), LIGHT_SIZE_IN_VBO);
    light_fillRenderer(&lightRenderer, bufferData, 1);
    light_render(&lightRenderer, 0);
    free(bufferData);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glFrontFace(GL_CCW);

    glDisable(GL_BLEND);


    //render forward scene --------------------------------------------------------------------------------

    //light kuba
    glUseProgram(forwardPassShader.id);
    glUniformMatrix4fv(glGetUniformLocation(forwardPassShader.id, "projection"), 1, GL_FALSE, projection.data);
    glUniformMatrix4fv(glGetUniformLocation(forwardPassShader.id, "view"), 1, GL_FALSE, view.data);
    for (unsigned int i = 1; i < lights->size; i++)
    {
        mat4 model = mat4_create(1.0f);
        model = mat4_translate(model, ((light*)vector_get(lights, i))->position);
        model = mat4_scale(model, vec3_create(0.125f));
        glUniformMatrix4fv(glGetUniformLocation(forwardPassShader.id, "model"), 1, GL_FALSE, model.data);
        glUniform3fv(glGetUniformLocation(forwardPassShader.id, "lightColor"), 1, (float*)&(((light*)vector_get(lights, i))->colour.x));
        render_cube();
    }

    //switch to default fbo ------------------------------------------------------------------------
    glViewport(0, 0, window_getWidth(), window_getHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);//default framebuffer
    //glClearColor(0, 0.04f, 0.6f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);

    //skybox
    glDisable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureHandler_getTexture(TEXTURE_SKYBOX));
    glUseProgram(skyboxShader.id);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.id, "pvm"), 1, GL_FALSE, mat4_multiply(pv, mat4_create2((float[]) { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, cum->position.x, cum->position.y, cum->position.z, 1 })).data);
    glBindVertexArray(skyboxMesh.vao);
    glDrawElements(GL_TRIANGLES, skyboxMesh.indexCount, GL_UNSIGNED_INT, 0);

    glFrontFace(GL_CCW);

    //sun (ide lehet, hogy kell majd blend)
    sun_render(&szunce, cum, &projection);
    glEnable(GL_DEPTH_TEST);

    // draw results to screen -----------------------------------------------------------------------------
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rendor.endBuffer.colorBuffer);

    glUseProgram(finalPassShader.id);
    glBindVertexArray(rectangleVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisable(GL_BLEND);

    //render 2d stuff (only text yet)
    //everything is inside render_text like use shader bing VAO etc. (inefficient but good enough for now)
    render_text(f, "amogus", 10, 10, 1);
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

chunkManager cm;
void init_kuba()
{
    cm = chunkManager_create(69, 4);
}

void end_kuba() {
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
    glUseProgram(textShader.id);
    mat4 projection2D = mat4_ortho(0, window_getWidth(), 0, window_getHeight(), -1, 1);
    glUniformMatrix4fv(glGetUniformLocation(textShader.id, "projection"), 1, GL_FALSE, projection2D.data);
    glUniform3f(glGetUniformLocation(textShader.id, "textColor"), 1, 1, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);
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
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos,     ypos + h,   0.0f, 0.0f },
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

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void render_cube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

double lerp(double a, double b, double f)
{
    return a + f * (b - a);
}