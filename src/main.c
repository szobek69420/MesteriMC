#define _CRT_SECURE_NO_WARNINGS

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <pthread.h>


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
#include "mesh/player_mesh/player_mesh.h"

#include "ui/font_handler/font_handler.h"
#include "ui/text/text_renderer.h"

#include "post_processing/lens_flare/flare.h"

#include "glm2/mat4.h"
#include "glm2/vec3.h"
#include "glm2/mat3.h"

#include "utils/list.h"
#include "utils/vector.h"
#include "utils/lista.h"


#define CLIP_NEAR 0.1f
#define CLIP_FAR 200.0f

#define PHYSICS_UPDATE 0.02f
#define GENERATION_UPDATE 0.005f

//global variable
GLFWwindow* window;
int windowShouldClose = 0;

pthread_t thread_render;
pthread_t thread_generation;
pthread_t thread_physics;

pthread_t mutex_input;//mutex for glfwPollEvents
pthread_t mutex_swap;//mutex for glfwSwapBuffers
pthread_t mutex_exit;//mutex for glfwWindowShouldClose check
int shouldPoll, shouldSwap, shouldExit;

pthread_mutex_t mutex_window;//for window calls (like window_getHeight())

chunkManager cm;
pthread_mutex_t mutex_cm;

camera cum;
pthread_mutex_t mutex_cum;

playerMesh pm;
pthread_mutex_t mutex_pm;

renderer rendor;
shader shadowChunkShader;
shader shadowPlayerShader;
shader geometryPassShader;
shader ssaoShader;
shader ssaoBlurShader;
shader lightingPassShader;
shader waterShader;
shader forwardPassShader;
shader finalPassShader;
shader fxaaShader;
shader skyboxShader; mesh skyboxMesh;

textRenderer tr;
pthread_mutex_t mutex_tr;
font f;

unsigned int rectangleVAO;//a deferred resz hasznalja a kepernyo atrajzolasahoz
unsigned int rectangleVBO;

vec3 ssaoKernel[64];
unsigned int noiseTexture;

vector* lights;
light sunTzu;

light_renderer lightRenderer;

sun szunce;

flare lensFlare;

//function prototypes
GLFWwindow* init_window(const char* name, int width, int height);
void handle_event(event e);

void* loop_render(void* arg);
void* loop_generation(void* arg);
void* loop_physics(void* arg);

void init_renderer();
void end_renderer();

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
    window_setWidth(1300);
    window_setHeight(800);
    window = init_window("amogus", window_getWidth(), window_getHeight());

    event_queue_init();
    input_init();

    tr = textRenderer_create(window_getWidth(), window_getHeight());
    fontHandler_init();
    f = fontHandler_loadFont("../assets/fonts/Monocraft.ttf", 48);

    cum = camera_create(vec3_create2(0, 50, 0), vec3_create2(0, 1, 0), 0, 0, 90, 40, 0.2);

    cm = chunkManager_create(69, 5);
    init_renderer();

    textureHandler_importTextures();

    glfwMakeContextCurrent(NULL);

    //threading
    shouldPoll = 0;
    shouldSwap = 0;
    shouldExit = 0;

    pthread_mutex_init(&mutex_input, NULL);
    pthread_mutex_init(&mutex_swap, NULL);
    pthread_mutex_init(&mutex_exit, NULL);

    pthread_mutex_init(&mutex_window, NULL);
    pthread_mutex_init(&mutex_cm, NULL);
    pthread_mutex_init(&mutex_pm, NULL);
    pthread_mutex_init(&mutex_cum, NULL);
    pthread_mutex_init(&mutex_tr, NULL);


    pthread_create(&thread_physics, NULL, loop_physics, NULL);
    pthread_create(&thread_generation, NULL, loop_generation, NULL);
    pthread_create(&thread_render, NULL, loop_render, NULL);

    while (69)
    {
        //check if should poll
        pthread_mutex_lock(&mutex_input);
        if (shouldPoll)
        {
            glfwPollEvents();
            shouldPoll = 0;
        }
        pthread_mutex_unlock(&mutex_input);

        //check if it should quit
        int shouldExit2 = 0;
        pthread_mutex_lock(&mutex_exit);
        shouldExit2 = shouldExit;
        pthread_mutex_unlock(&mutex_exit);
        if (shouldExit2)
            break;
    }

    pthread_join(thread_generation, NULL);
    pthread_join(thread_render, NULL);
    pthread_join(thread_physics, NULL);

    
    pthread_mutex_destroy(&mutex_input);
    pthread_mutex_destroy(&mutex_swap);
    pthread_mutex_destroy(&mutex_exit);

    pthread_mutex_destroy(&mutex_window);
    pthread_mutex_destroy(&mutex_cm);
    pthread_mutex_destroy(&mutex_pm);
    pthread_mutex_destroy(&mutex_cum);
    pthread_mutex_destroy(&mutex_tr);


    glfwMakeContextCurrent(window);

    chunkManager_destroy(&cm);
    textRenderer_destroy(&tr);
    textureHandler_destroyTextures();
    fontHandler_close();
    end_renderer();

    glfwTerminate();
    return 69;
}

void* loop_render(void* arg)
{
    glfwMakeContextCurrent(window);

    int windowWidth, windowHeight, lastWindowWidth=window_getWidth(), lastWindowHeight=window_getHeight(), shouldChangeSize=0;
    float windowAspectXY;

    const char * vendor = glGetString(GL_VENDOR);
    const char * renderer = glGetString(GL_RENDERER);

    float deltaTime;
    float lastFrame = glfwGetTime();
    float lastSecond = 0;//az fps szamolashoz
    int framesInLastSecond = 0;

    camera cum_render;
    while (69)
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
       
        //get window info
        pthread_mutex_lock(&mutex_window);
        windowWidth = window_getWidth();
        windowHeight = window_getHeight();
        windowAspectXY = window_getAspect();
        pthread_mutex_unlock(&mutex_window);

        if (windowWidth != lastWindowWidth || windowHeight != lastWindowHeight)
            shouldChangeSize = 69;
        lastWindowWidth = windowWidth;
        lastWindowHeight = windowHeight;

        //get cum info
        pthread_mutex_lock(&mutex_cum);
        cum_render = cum;
        pthread_mutex_unlock(&mutex_cum);

        //update chunk mesh data in gpu
        pthread_mutex_lock(&mutex_cm);
        chunkManager_updateMesh(&cm);
        chunkManager_updateMesh(&cm);
        chunkManager_updateMesh(&cm);
        chunkManager_updateMesh(&cm);
        chunkManager_updateMesh(&cm);
        pthread_mutex_unlock(&mutex_cm);

        //render------------------------------------------
        //matrices
        mat4 view = camera_getViewMatrix(&cum_render);
        mat4 projection = mat4_perspective(cum_render.fov, windowAspectXY, CLIP_NEAR, CLIP_FAR);
        mat4 pv = mat4_multiply(projection, view);
        mat4 projectionInverse = mat4_inverse(projection);
        mat3 viewNormal = mat3_createFromMat(view);

        mat4 shadowViewProjection = mat4_multiply(
            mat4_ortho(-70, 70, -70, 70, 1, 200),
            mat4_lookAt(
                vec3_sum(cum_render.position, vec3_create2(szunce.direction.x * 100, szunce.direction.y * 100, szunce.direction.z * 100)),
                vec3_create2(-1 * szunce.direction.x, -1 * szunce.direction.y, -1 * szunce.direction.z),
                vec3_create2(0, 1, 0)
            )
        );
        mat4 shadowLightMatrix = mat4_multiply(shadowViewProjection, mat4_inverse(view));//from the camera's view space to the suns projection space

        //shadow
        glViewport(0, 0, RENDERER_SHADOW_RESOLUTION, RENDERER_SHADOW_RESOLUTION);
        glBindFramebuffer(GL_FRAMEBUFFER, rendor.shadowBuffer.id);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);

        glDisable(GL_CULL_FACE);
        //glFrontFace(GL_CW);

        glUseProgram(shadowChunkShader.id);
        pthread_mutex_lock(&mutex_cm);
        chunkManager_drawShadow(&cm, &shadowChunkShader, &shadowViewProjection);
        pthread_mutex_unlock(&mutex_cm);

        glUseProgram(shadowPlayerShader.id);
        glUniformMatrix4fv(glGetUniformLocation(shadowPlayerShader.id, "lightMatrix"), 1, GL_FALSE, shadowViewProjection.data);
        pthread_mutex_lock(&mutex_pm);
        playerMesh_render(&pm, &shadowPlayerShader);
        pthread_mutex_unlock(&mutex_pm);

        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);


        //prepare gbuffer fbo
        glViewport(0, 0, RENDERER_WIDTH, RENDERER_HEIGHT);
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

        pthread_mutex_lock(&mutex_cm);
        chunkManager_drawTerrain(&cm, &geometryPassShader, &cum, &projection);
        pthread_mutex_unlock(&mutex_cm);

        //copy depth buffer
        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, rendor.gBuffer.id);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rendor.endBuffer.id);
        glBlitFramebuffer(0, 0, RENDERER_WIDTH, RENDERER_HEIGHT, 0, 0, RENDERER_WIDTH, RENDERER_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        //ssao pass ------------------------------------------------------------------------------------------
        // generate SSAO texture
        /*glBindFramebuffer(GL_FRAMEBUFFER, rendor.ssaoBuffer.idColor);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(ssaoShader.id);
        // Send kernel + rotation
        for (unsigned int i = 0; i <64; ++i)
        {
            static char buffer[20];
            sprintf(buffer, "samples[%d]", i);
            glUniform3f(glGetUniformLocation(ssaoShader.id, buffer), 1, ssaoKernel[i].x, ssaoKernel[i].y, ssaoKernel[i].z);
        }
        glUniformMatrix4fv(glGetUniformLocation(ssaoShader.id, "projection"), 1, GL_FALSE, projection.data);
        glUniformMatrix4fv(glGetUniformLocation(ssaoShader.id, "projection_inverse"), 1, GL_FALSE, projectionInverse.data);
        glUniformMatrix3fv(glGetUniformLocation(ssaoShader.id, "viewForDirectional"), 1, GL_FALSE, viewNormal.data);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rendor.gBuffer.normal);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, rendor.gBuffer.depthBuffer);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glBindVertexArray(rectangleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // blur SSAO texture to remove noise
        glBindFramebuffer(GL_FRAMEBUFFER, rendor.ssaoBuffer.idBlur);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(ssaoBlurShader.id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rendor.ssaoBuffer.colorBuffer);
        glBindVertexArray(rectangleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);*/

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
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);//az alpha azert GL_ONE/GL_ZERO, hogy felulirja a korabbi erteket az uj ertek (ugyis ugyanaz, de igy nem latszik a korvonala a gomboknek a distance fog-ban)
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
        //light_setPosition((light*)vector_get(lights, 0), cum->position);
        float* bufferData = (float*)malloc(lights->size * LIGHT_SIZE_IN_VBO);
        light* tempLight;
        for (unsigned int i = 0; i < lights->size; i++)
        {
            tempLight = (light*)vector_get(lights, i);
            memcpy(bufferData + i * LIGHT_FLOATS_IN_VBO, &(tempLight->position.x), LIGHT_SIZE_IN_VBO);
        }

        glUniform1i(glGetUniformLocation(lightingPassShader.id, "shadowOn"), 0);

        light_fillRenderer(&lightRenderer, bufferData, lights->size);
        light_render(&lightRenderer, 69);
        free(bufferData);

        //directional (sun)
        glUniform1i(glGetUniformLocation(lightingPassShader.id, "shadowOn"), 69);
        glUniformMatrix4fv(glGetUniformLocation(lightingPassShader.id, "shadow_lightMatrix"), 1, GL_FALSE, shadowLightMatrix.data);

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
        for (unsigned int i = 0; i < lights->size; i++)
        {
            mat4 model = mat4_create(1.0f);
            model = mat4_translate(model, ((light*)vector_get(lights, i))->position);
            model = mat4_scale(model, vec3_create(0.125f));
            glUniformMatrix4fv(glGetUniformLocation(forwardPassShader.id, "model"), 1, GL_FALSE, model.data);
            glUniform3fv(glGetUniformLocation(forwardPassShader.id, "lightColor"), 1, (float*)&(((light*)vector_get(lights, i))->colour.x));
            render_cube();
        }

        //walter
        glUseProgram(waterShader.id);

        glUniform1f(glGetUniformLocation(waterShader.id, "uvOffset"), 0.03f * glfwGetTime());

        glUniform3f(glGetUniformLocation(waterShader.id, "waterColourDeep"), 0, 0.0627f, 0.8f);
        glUniform3f(glGetUniformLocation(waterShader.id, "waterColourShallow"), 0, 0.8627f, 0.8941f);

        glUniform3f(glGetUniformLocation(waterShader.id, "sun.position"), sunTzu.position.x, sunTzu.position.y, sunTzu.position.z);
        glUniform3f(glGetUniformLocation(waterShader.id, "sun.colour"), sunTzu.colour.x, sunTzu.colour.y, sunTzu.colour.z);
        glUniform3f(glGetUniformLocation(waterShader.id, "sun.attenuation"), sunTzu.attenuation.x, sunTzu.attenuation.y, sunTzu.attenuation.z);

        glUniform3f(glGetUniformLocation(waterShader.id, "cameraPos"), cum_render.position.x, cum_render.position.y, cum_render.position.z);
        glUniformMatrix4fv(glGetUniformLocation(waterShader.id, "view"), 1, GL_FALSE, view.data);
        glUniformMatrix4fv(glGetUniformLocation(waterShader.id, "projection"), 1, GL_FALSE, projection.data);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_WATER_NORMAL));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_WATER_DUDV));

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, rendor.gBuffer.depthBuffer);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        pthread_mutex_lock(&mutex_cm);
        chunkManager_drawWalter(&cm, &waterShader, &cum, &projection);
        pthread_mutex_unlock(&mutex_cm);

        glDisable(GL_BLEND);

        //get lens flare data
        flare_queryQueryResult(&lensFlare);
        flare_query(&lensFlare, &pv, cum_render.position, sunTzu.position, 1.0f / windowAspectXY);
        //switch to screen fbo ------------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, rendor.screenBuffer.id);

        //skybox
        glDisable(GL_DEPTH_TEST);
        glFrontFace(GL_CW);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_SKY_GRADIENT));

        glUseProgram(skyboxShader.id);
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.id, "pvm"), 1, GL_FALSE, mat4_multiply(pv, mat4_create2((float[]) { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, cum_render.position.x, cum_render.position.y, cum_render.position.z, 1 })).data);
        glBindVertexArray(skyboxMesh.vao);
        glDrawElements(GL_TRIANGLES, skyboxMesh.indexCount, GL_UNSIGNED_INT, 0);

        glFrontFace(GL_CCW);

        // draw the content of endfbo to screenfbo -----------------------------------------------------------------------------
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

        //lens flare
        flare_render(&lensFlare, &pv, cum_render.position, sunTzu.position, 1.0f / windowAspectXY);

        //switch to default fbo ------------------------------------------------------------------------
        glViewport(0, 0, windowWidth, windowHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);//default framebuffer

        //fxaa
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rendor.screenBuffer.colorBuffer);

        glUseProgram(fxaaShader.id);
        glBindVertexArray(rectangleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //render 2d stuff (only text yet)
        pthread_mutex_lock(&mutex_tr);
        
        if (shouldChangeSize)
        {
            textRenderer_setSize(&tr, windowWidth, windowHeight);
        }

        textRenderer_render(&tr, &f, vendor, windowWidth-0.5f*fontHandler_calculateTextLength(&f, vendor) - 15, windowHeight-34, 0.5);
        textRenderer_render(&tr, &f, renderer, windowWidth - 0.5f * fontHandler_calculateTextLength(&f, renderer) - 15, windowHeight - 69, 0.5);


        static char buffer[50];
        sprintf(buffer, "FPS: %.0f", 1.0 / deltaTime);
        textRenderer_render(&tr, &f, buffer, 15, windowHeight - 34, 0.5);
        sprintf(buffer, "Pos: %d %d %d", (int)cum_render.position.x, (int)cum_render.position.y, (int)cum_render.position.z);
        textRenderer_render(&tr, &f, buffer, 15, windowHeight - 69, 0.5);

        pthread_mutex_unlock(&mutex_tr);

        glfwSwapBuffers(window);

        
        //query shouldExit
        int shouldExit2 = 0;
        pthread_mutex_lock(&mutex_exit);
        shouldExit2 = shouldExit;
        pthread_mutex_unlock(&mutex_exit);
        if (shouldExit2)
            break;
    }

    glfwMakeContextCurrent(NULL);

    return NULL;
}

void* loop_generation(void* arg)
{
    float deltaTime;
    float lastFrame = glfwGetTime();
    float lastCameraUpdate = 0;
    camera cum_generation;

    while (69)
    {
        float currentTime = glfwGetTime();
        while (currentTime - lastFrame < GENERATION_UPDATE)
            currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        if (currentTime - lastCameraUpdate>0.5f)//dont refresh camera every frame because it slows the other threads down
        {
            pthread_mutex_lock(&mutex_cum);
            cum_generation = cum;
            lastCameraUpdate = currentTime;
            pthread_mutex_unlock(&mutex_cum);
        }

        int chunkX, chunkY, chunkZ;
        chunk_getChunkFromPos(vec3_create2(cum_generation.position.x + CHUNK_WIDTH * 0.5f, cum_generation.position.y + CHUNK_HEIGHT * 0.5f, cum_generation.position.z + CHUNK_WIDTH * 0.5f), &chunkX, &chunkY, &chunkZ);
        pthread_mutex_lock(&mutex_cm);
        chunkManager_searchForUpdates(&cm, chunkX, chunkY, chunkZ);
        pthread_mutex_unlock(&mutex_cm);
        chunkManager_update(&cm,&mutex_cm);
        chunkManager_update(&cm,&mutex_cm);

        
        //query shouldExit
        int shouldExit2 = 0;
        pthread_mutex_lock(&mutex_exit);
        shouldExit2 = shouldExit;
        pthread_mutex_unlock(&mutex_exit);
        if (shouldExit2)
            break;
    }
    return NULL;
}

void* loop_physics(void* arg)
{

    float deltaTime;
    float lastFrame = glfwGetTime();
    vec3 previousCumPosition = cum.position;

    while (69)
    {
        float currentTime = glfwGetTime();
        while (currentTime - lastFrame < PHYSICS_UPDATE)
            currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        //update
        input_update();
        event e;
        while ((e = event_queue_poll()).type != NONE)
            handle_event(e);

        pthread_mutex_lock(&mutex_cum);
        previousCumPosition = cum.position;
        camera_update(&cum, deltaTime);
        pthread_mutex_unlock(&mutex_cum);

        //player animation
        pthread_mutex_lock(&mutex_pm);
        pm.position = (vec3){ cum.position.x, cum.position.y - 1.6f, cum.position.z };
        pm.rotX = cum.pitch;
        pm.rotY = cum.yaw;
        pm.rotHeadX = cum.pitch;
        playerMesh_animate(&pm, (vec3_sqrMagnitude((vec3) { previousCumPosition.x - cum.position.x, 0, previousCumPosition.z - cum.position.z }) > 0.00001f) ? PLAYER_MESH_ANIMATION_WALK : PLAYER_MESH_ANIMATION_IDLE, deltaTime);
        playerMesh_calculateOuterModelMatrix(&pm);
        playerMesh_calculateInnerModelMatrices(&pm);
        pthread_mutex_unlock(&mutex_pm);
        

        pthread_mutex_lock(&mutex_input);
        shouldPoll = 69;
        pthread_mutex_unlock(&mutex_input);

        int shouldExit2 = 0;
        int shouldPoll2 = 0;
        while (69)
        {
            //query shouldExit
            pthread_mutex_lock(&mutex_exit);
            shouldExit2 = shouldExit;
            pthread_mutex_unlock(&mutex_exit);

            //query shouldPoll
            pthread_mutex_lock(&mutex_input);
            shouldPoll2 = shouldPoll;
            pthread_mutex_unlock(&mutex_input);

            if (shouldExit2 || !shouldPoll2)
                break;
        }
        if (shouldExit2)
            break;
    }


    return NULL;
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
        pthread_mutex_lock(&mutex_window);
        window_setWidth(e.data.window_resize.width);
        window_setHeight(e.data.window_resize.height);
        pthread_mutex_unlock(&mutex_window);
        //azert nem itt allitom at a text_renderer meretet, mert ez nem az opengl szal
        break;
    default:
        input_handle_event(e);
        if (e.type == KEY_PRESSED && e.data.key_pressed.key_code == GLFW_KEY_ESCAPE)
        {
            pthread_mutex_lock(&mutex_exit);
            shouldExit = 69;
            pthread_mutex_unlock(&mutex_exit);
        }
        break;
    }
}


void init_renderer()
{
    //rendor = renderer_create(window_getWidth(), window_getHeight());
    rendor = renderer_create(RENDERER_WIDTH, RENDERER_HEIGHT);

    //shaders
    shadowChunkShader = shader_import(
        "../assets/shaders/renderer/shadow/shader_shadow_chunk.vag",
        "../assets/shaders/renderer/shadow/shader_shadow.fag",
        NULL);

    shadowPlayerShader = shader_import(
        "../assets/shaders/renderer/shadow/shader_shadow_player.vag",
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

    waterShader = shader_import(
        "../assets/shaders/renderer/water/shader_forward_water.vag",
        "../assets/shaders/renderer/water/shader_forward_water.fag",
        NULL
    );
    glUseProgram(waterShader.id);
    glUniform1i(glGetUniformLocation(waterShader.id, "texture_normal"), 0);
    glUniform1i(glGetUniformLocation(waterShader.id, "texture_dudv"), 1);
    glUniform1i(glGetUniformLocation(waterShader.id, "texture_depth_geometry"), 2);
    glUniform1i(glGetUniformLocation(waterShader.id, "texture_depth_shadow"), 3);
    glUniform1f(glGetUniformLocation(waterShader.id, "fogStart"), 150);
    glUniform1f(glGetUniformLocation(waterShader.id, "fogEnd"), 160);
    glUniform1f(glGetUniformLocation(waterShader.id, "fogHelper"), 1.0f / (160 - 150));
    glUniform1f(glGetUniformLocation(waterShader.id, "projectionNear"), CLIP_NEAR);
    glUniform1f(glGetUniformLocation(waterShader.id, "projectionFar"), CLIP_FAR);
    glUniform1f(glGetUniformLocation(waterShader.id, "onePerScreenWidth"), 1.0f / RENDERER_WIDTH);
    glUniform1f(glGetUniformLocation(waterShader.id, "onePerScreenHeight"), 1.0f / RENDERER_HEIGHT);

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
    glUniform1f(glGetUniformLocation(ssaoShader.id, "projectionNear"), CLIP_NEAR);
    glUniform1f(glGetUniformLocation(ssaoShader.id, "projectionFar"), CLIP_FAR);

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
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogStart"), 150);
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogEnd"), 160);
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogHelper"), 1.0f/(160-150));
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "shadowStart"), 60);
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "shadowEnd"), 70);
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "shadowHelper"), 1.0f / (70 - 60));

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
    glUniform1f(glGetUniformLocation(finalPassShader.id, "exposure"), 0.2);

    fxaaShader = shader_import(
        "../assets/shaders/renderer/fxaa/shader_fxaa.vag",
        "../assets/shaders/renderer/fxaa/shader_fxaa.fag",
        NULL
    );
    glUseProgram(fxaaShader.id);
    glUniform1i(glGetUniformLocation(fxaaShader.id, "tex"), 0);
    glUniform2f(glGetUniformLocation(fxaaShader.id, "onePerResolution"), 1.0f/RENDERER_WIDTH, 1.0f/RENDERER_HEIGHT);

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

    //cull front faces
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //ssao shit
    // generate sample kernel
    // ----------------------
    for (unsigned int i = 0; i < 64; ++i)
    {
        ssaoKernel[i] = vec3_create2(
            (rand() % 1001) / 1000.0 * 2.0 - 1.0, //random between [-1.0 - 1.0]
            (rand() % 1001) / 1000.0 * 2.0 - 1.0, //random between [-1.0 - 1.0]
            (rand() % 1001) / 1000.0              //random between [ 0.0 - 1.0]
        );
        // scale samples s.t. they're more aligned to center of kernel
        ssaoKernel[i] = vec3_normalize(ssaoKernel[i]);
        double scale = i / 64.0;
        scale = lerp(0.1, 1.0, scale * scale);
        ssaoKernel[i] = vec3_scale(ssaoKernel[i], (rand() % 1001) / 1000.0 * scale); //random between [0.0 - 1.0] * scale
    }
    // generate noise texture
    // ----------------------
    vec3 ssaoNoise[16];
    for (unsigned int i = 0; i < 16; i++)
    {
        ssaoNoise[i] = vec3_create2(
            (rand() % 1001) / 1000.0 * 2.0 - 1.0, //random between [-1.0 - 1.0]
            (rand() % 1001) / 1000.0 * 2.0 - 1.0, //random between [-1.0 - 1.0]
            0.0f
        ); // rotate around z-axis (in tangent space)
    }
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise);
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
            vec3_create2(10, 0.1, 0.1)
            );
        vector_push_back(lights, lit);
    }

    sunTzu = light_create(
        vec3_create2(1, 0.97, 0.4),
        vec3_create2(0.6, 0.5, -0.8),
        vec3_create2(10, 0, 0)
    );

    //skybox
    skyboxShader = shader_import(
        "../assets/shaders/sky_procedural/shader_sky_procedural.vag",
        "../assets/shaders/sky_procedural/shader_sky_procedural.fag",
        NULL
    );
    glUseProgram(skyboxShader.id);
    glUniform1i(glGetUniformLocation(skyboxShader.id, "texture_gradient"), 0);
    glUniform3f(glGetUniformLocation(skyboxShader.id, "sunDirectionNormalized"), sunTzu.position.x, sunTzu.position.y, sunTzu.position.z);
    glUniform3f(glGetUniformLocation(skyboxShader.id, "sunColour"), 1,1,1);
    glUniform3f(glGetUniformLocation(skyboxShader.id, "skyColour0"), 0,0.8f,1.0f);
    glUniform3f(glGetUniformLocation(skyboxShader.id, "skyColour1"), 1, 1, 1);
    glUniform3f(glGetUniformLocation(skyboxShader.id, "skyColour2"), 0, 0.8f, 1.0f);
    glUseProgram(0);

    skyboxMesh = kuba_create();

    //sun
    szunce = sun_create();
    sun_setDirection(&szunce, vec3_create2(0.6, 1, -0.8));

    //lens flare
    lensFlare = flare_create(0.4f);

    //player mesh
    pm=playerMesh_create();
}

void end_renderer()
{
    renderer_destroy(rendor);

    shader_delete(&shadowChunkShader);
    shader_delete(&shadowPlayerShader);
    shader_delete(&geometryPassShader);
    shader_delete(&ssaoShader);
    shader_delete(&ssaoBlurShader);
    shader_delete(&lightingPassShader);
    shader_delete(&waterShader);
    shader_delete(&forwardPassShader);
    shader_delete(&finalPassShader);
    shader_delete(&fxaaShader);

    glDeleteVertexArrays(1, &rectangleVAO);
    glDeleteBuffers(1, &rectangleVBO);

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

    //lens flare
    flare_destroy(&lensFlare);

    //player mesh
    playerMesh_destroy(&pm);
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