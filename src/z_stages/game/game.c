#define _CRT_SECURE_NO_WARNINGS

#include "game.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <pthread.h>


#include "../stages.h"
#include "../../window/window.h"
#include "../../camera/camera.h"
#include "../../event/event_queue.h"
#include "../../input/input.h"
#include "../../shader/shader.h"
#include "../../texture_handler/texture_handler.h"

#include "../../world/chunk/chunk.h"
#include "../../world/chunk/chunkManager.h"
#include "../../world/sun/sun.h"

#include "../../renderer/framebuffer/framebuffer.h"
#include "../../renderer/light/light.h"
#include "../../mesh/sphere/sphere.h"
#include "../../mesh/kuba/kuba.h"
#include "../../mesh/player_mesh/player_mesh.h"

#include "../../renderer/block_selection/block_selection.h"

#include "../../ui/font_handler/font_handler.h"
#include "../../ui/text/text_renderer.h"
#include "../../ui/canvas/canvas.h"

#include "../../post_processing/lens_flare/flare.h"

#include "../../glm2/mat4.h"
#include "../../glm2/vec3.h"
#include "../../glm2/mat3.h"

#include "../../physics/collider/collider.h"
#include "../../physics/physics_system/physics_system.h"

#include "../../utils/list.h"
#include "../../utils/vector.h"
#include "../../utils/lista.h"


#define CLIP_NEAR 0.1f
#define CLIP_FAR 200.0f

#define PHYSICS_UPDATE 0.02f
#define PHYSICS_STEPS_PER_UPDATE 100
#define GENERATION_UPDATE 0.005f
#define CHUNK_UPDATES_PER_GENERATION_UPDATE 2

//global variable
GLFWwindow* window;
int windowShouldClose = 0;

pthread_t thread_render;
pthread_t thread_generation;
pthread_t thread_physics;

pthread_t mutex_input;//mutex for glfwPollEvents
pthread_t mutex_swap;//mutex for glfwSwapBuffers
pthread_t mutex_exit;//mutex for glfwWindowShouldClose check
int shouldPoll, shouldSwap, exitStatus;
enum {
    EXIT_STATUS_REMAIN, EXIT_STATUS_CLOSE_PROGRAM, EXIT_STATUS_RETURN_TO_MENU
};

pthread_mutex_t mutex_window;//for window calls (like window_getHeight())

physicsSystem ps;
raycastHit rh;
int raycastChunkX, raycastChunkY, raycastChunkZ;
int raycastBlockX, raycastBlockY, raycastBlockZ;
int raycastFound = 0;

chunkManager cm;
pthread_mutex_t mutex_cm;
int chunkUpdatesInLastSecond = 0;

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
shader bloomFilterShader; shader bloomDownsampleShader;
shader finalPassShader;
shader fxaaShader;
shader skyboxShader; mesh skyboxMesh;

canvas* vaszon;
pthread_mutex_t mutex_vaszon;
int vaszon_fps;
int vaszon_pos;
int vaszon_rot;
int vaszon_render_distance;
int vaszon_chunk_updates;
int vaszon_chunks_loaded;
int vaszon_chunks_rendered;

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

void init_canvas();
void end_canvas();

unsigned int cubeVAO = 0, cubeVBO = 0;
void init_cube();
void end_cube();

double lerp(double a, double b, double f);

//glfw callbacks
void window_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void game(void* w, int* currentStage)
{
    //callbacks
    window = (GLFWwindow*)w;
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    textureHandler_importTextures(TEXTURE_IN_GAME);

    event_queue_init();
    input_init();


    fontHandler_init();
    init_canvas();

    cum = camera_create(vec3_create2(0, 50, 0), vec3_create2(0, 1, 0), 0, 0, 90, 40, 0.2);

    ps = physicsSystem_create();

    cm = chunkManager_create(69, 5, &ps);
    chunk_resetGenerationInfo();
    init_renderer();

    init_cube();//egyszer majd ki kene szedni

    blockSelection_init();

    glfwMakeContextCurrent(NULL);

    //threading
    shouldPoll = 0;
    shouldSwap = 0;
    exitStatus = EXIT_STATUS_REMAIN;

    pthread_mutex_init(&mutex_input, NULL);
    pthread_mutex_init(&mutex_swap, NULL);
    pthread_mutex_init(&mutex_exit, NULL);

    pthread_mutex_init(&mutex_window, NULL);
    pthread_mutex_init(&mutex_cm, NULL);
    pthread_mutex_init(&mutex_pm, NULL);
    pthread_mutex_init(&mutex_cum, NULL);
    pthread_mutex_init(&mutex_vaszon, NULL);


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
        shouldExit2 = exitStatus;
        pthread_mutex_unlock(&mutex_exit);
        if (shouldExit2!=EXIT_STATUS_REMAIN)
        {
            if (shouldExit2 == EXIT_STATUS_RETURN_TO_MENU)
                *currentStage = STAGE_MAIN_MENU;
            else
                *currentStage = STAGE_QUIT;
            break;
        }
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
    pthread_mutex_destroy(&mutex_vaszon);


    glfwMakeContextCurrent(window);

    chunkManager_destroy(&cm);
    physicsSystem_destroy(&ps);
    textureHandler_destroyTextures(TEXTURE_IN_GAME);
    end_canvas();
    fontHandler_close();
    end_renderer();

    end_cube();

    blockSelection_close();

    int chunksGenerated, chunksDestroyed;
    chunk_getGenerationInfo(&chunksGenerated, &chunksDestroyed);
    printf("chunks generated: %d, chunks destroyed: %d\n", chunksGenerated, chunksDestroyed);
    
    return 69;
}

void* loop_render(void* arg)
{
    glfwMakeContextCurrent(window);

    int windowWidth, windowHeight, lastWindowWidth = window_getWidth(), lastWindowHeight = window_getHeight(), shouldChangeSize = 0;
    float windowAspectXY;

    float deltaTime;
    float lastFrame = glfwGetTime();
    float lastSecond = 0;//az fps szamolashoz
    int frameCounter = 0;
    int framesInLastInterval = 0;

    int loadedChunks = 0;

    camera cum_render;
    while (69)
    {
        deltaTime = glfwGetTime() - lastFrame;
        lastFrame = glfwGetTime();
        lastSecond += deltaTime;
        frameCounter++;
        if (lastSecond > 0.2857f)
        {
            //printf("FPS: %d\n", framesInLastSecond);
            //printf("Pos: %d %d %d\n\n", (int)cum.position.x, (int)cum.position.y, (int)cum.position.z);
            lastSecond = 0;
            framesInLastInterval = frameCounter;
            frameCounter = 0;
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

        loadedChunks = cm.loadedChunks.size;
        pthread_mutex_unlock(&mutex_cm);

        //render------------------------------------------
        //matrices
        mat4 view = camera_getViewMatrix(&cum_render);
        mat4 projection = mat4_perspective(cum_render.fov, windowAspectXY, CLIP_NEAR, CLIP_FAR);
        mat4 pv = mat4_multiply(projection, view);
        mat4 projectionInverse = mat4_inverse(projection);
        mat3 viewNormal = mat3_createFromMat(view);

        mat4 shadowViewProjection = mat4_multiply(
            mat4_ortho(-150, 150, -150, 150, 1, 200),
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
        int renderedChunks = chunkManager_drawTerrain(&cm, &geometryPassShader, &cum, &projection);
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
            // render Cube
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }

        //raycast
        do {
            vec3 sPos = (vec3){ CHUNK_WIDTH * raycastChunkX + raycastBlockX + 0.5f, CHUNK_HEIGHT * raycastChunkY + raycastBlockY + 0.5f, CHUNK_WIDTH * raycastChunkZ + raycastBlockZ + 0.5f };
            vec3 sSize = vec3_create2(1.005f, 1.005f, 1.005f);
            blockSelection_render(sPos, sSize, &pv);
        } while (0);

        //walter
        glUseProgram(waterShader.id);

        glUniform1f(glGetUniformLocation(waterShader.id, "uvOffset"), 0.03f * glfwGetTime());

        glUniform3f(glGetUniformLocation(waterShader.id, "waterColourDeep"), 0, 0.0627f, 0.8f);
        glUniform3f(glGetUniformLocation(waterShader.id, "waterColourShallow"), 0, 0.8627f, 0.8941f);

        glUniformMatrix4fv(glGetUniformLocation(waterShader.id, "projectionInverse"), 1, GL_FALSE, projectionInverse.data);

        glUniformMatrix4fv(glGetUniformLocation(waterShader.id, "shadow_lightMatrix"), 1, GL_FALSE, shadowLightMatrix.data);
        glUniform1i(glGetUniformLocation(waterShader.id, "shadowOn"), 69);

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

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, rendor.shadowBuffer.depthBuffer);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        pthread_mutex_lock(&mutex_cm);
        chunkManager_drawWalter(&cm, &waterShader, &cum, &projection);
        pthread_mutex_unlock(&mutex_cm);

        glDisable(GL_BLEND);

        //get lens flare data
        flare_queryQueryResult(&lensFlare);
        flare_query(&lensFlare, &pv, cum_render.position, sunTzu.position, 1.0f / windowAspectXY);

        //prepare bloom--------------------------------------------------------------------------------
        int viewportWidth = RENDERER_WIDTH/2, viewportHeight = RENDERER_HEIGHT/2;
        glViewport(0, 0, viewportWidth, viewportHeight);

        glBindVertexArray(rectangleVAO);
        glActiveTexture(GL_TEXTURE0);

        //filter dark areas
        glBindFramebuffer(GL_FRAMEBUFFER, rendor.bloomBuffers[0].id);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, rendor.endBuffer.colorBuffer);
        
        glUseProgram(bloomFilterShader.id);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        //downsample
        glUseProgram(bloomDownsampleShader.id);
        unsigned int loc = glGetUniformLocation(bloomDownsampleShader.id, "radius");
        for (int i = 1; i < RENDERER_KAWASAKI_FBO_COUNT; i++)
        {
            glViewport(0, 0, viewportWidth, viewportHeight);

            glBindFramebuffer(GL_FRAMEBUFFER, rendor.bloomBuffers[i].id);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            glBindTexture(GL_TEXTURE_2D, rendor.bloomBuffers[i-1].colorBuffer);
            glUniform2f(loc, (float)(i)/RENDERER_WIDTH, (float)(i)/RENDERER_HEIGHT);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            viewportWidth /= 2;
            viewportHeight /= 2;
        }

        //switch to screen fbo ------------------------------------------------------------------------
        glViewport(0, 0, RENDERER_WIDTH, RENDERER_HEIGHT);
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

        for (int i = GL_TEXTURE1; i < GL_TEXTURE1 + RENDERER_KAWASAKI_FBO_COUNT-1; i++)//-1, mert az elso bloombufferbol nincs mintavetelezes, mert azon nincs blur effekt
        {
            glActiveTexture(i);
            glBindTexture(GL_TEXTURE_2D, rendor.bloomBuffers[i-GL_TEXTURE1+1].colorBuffer);
        }

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
        pthread_mutex_lock(&mutex_vaszon);

        if (shouldChangeSize)
        {
            canvas_setSize(vaszon, windowWidth, windowHeight);
        }

        static char buffer[50];

        sprintf(buffer, "fps: %d", (int)(3.5f * framesInLastInterval));
        canvas_setTextText(vaszon, vaszon_fps, buffer);

        sprintf(buffer, "position: %d %d %d", (int)cum_render.position.x, (int)cum_render.position.y, (int)cum_render.position.z);
        canvas_setTextText(vaszon, vaszon_pos, buffer);

        sprintf(buffer, "rotation: %.f %.f", cum_render.pitch, cum_render.yaw);
        canvas_setTextText(vaszon, vaszon_rot, buffer);

        sprintf(buffer, "render distance: %d", cm.renderDistance);
        canvas_setTextText(vaszon, vaszon_render_distance, buffer);

        sprintf(buffer, "chunk updates: %d/s", chunkUpdatesInLastSecond);
        canvas_setTextText(vaszon, vaszon_chunk_updates, buffer);

        sprintf(buffer, "loaded chunks: %d", loadedChunks);
        canvas_setTextText(vaszon, vaszon_chunks_loaded, buffer);

        sprintf(buffer, "rendered chunks: %d", renderedChunks);
        canvas_setTextText(vaszon, vaszon_chunks_rendered, buffer);


        double mouseX=0, mouseY=0;
        input_get_mouse_position(&mouseX, &mouseY);
        canvas_render(vaszon, mouseX, mouseY, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT));

        pthread_mutex_unlock(&mutex_vaszon);

        glfwSwapBuffers(window);


        //query shouldExit
        int shouldExit2 = 0;
        pthread_mutex_lock(&mutex_exit);
        shouldExit2 = exitStatus;
        pthread_mutex_unlock(&mutex_exit);
        if (shouldExit2!=EXIT_STATUS_REMAIN)
            break;
    }

    glfwMakeContextCurrent(NULL);

    return NULL;
}

void* loop_generation(void* arg)
{
    float deltaTime;
    float lastFrame = glfwGetTime();
    float lastCameraUpdate = -1;
    float lastChunkUpdateUpdate = -1; int chunkUpdates = 0;
    camera cum_generation;

    while (69)
    {
        float currentTime = glfwGetTime();
        while (currentTime - lastFrame < GENERATION_UPDATE)
            currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        if (currentTime - lastCameraUpdate > 0.5f)//dont refresh camera every frame because it slows the other threads down
        {
            pthread_mutex_lock(&mutex_cum);
            cum_generation = cum;
            lastCameraUpdate = currentTime;
            pthread_mutex_unlock(&mutex_cum);
        }
        if (currentTime - lastChunkUpdateUpdate > 1)
        {
            lastChunkUpdateUpdate = currentTime;
            chunkUpdatesInLastSecond = chunkUpdates;
            chunkUpdates = 0;
        }

        int chunkX, chunkY, chunkZ;
        chunk_getChunkFromPos(vec3_create2(cum_generation.position.x + CHUNK_WIDTH * 0.5f, cum_generation.position.y + CHUNK_HEIGHT * 0.5f, cum_generation.position.z + CHUNK_WIDTH * 0.5f), &chunkX, &chunkY, &chunkZ);
        pthread_mutex_lock(&mutex_cm);
        chunkManager_searchForUpdates(&cm, chunkX, chunkY, chunkZ);
        pthread_mutex_unlock(&mutex_cm);

        for (int i = 0; i < CHUNK_UPDATES_PER_GENERATION_UPDATE; i++)
        {
            if (cm.pendingUpdates.size > 0)
            {
                chunkManager_update(&cm, &mutex_cm);
                chunkUpdates++;
            }
        }


        //query shouldExit
        int shouldExit2 = 0;
        pthread_mutex_lock(&mutex_exit);
        shouldExit2 = exitStatus;
        pthread_mutex_unlock(&mutex_exit);
        if (shouldExit2!=EXIT_STATUS_REMAIN)
            break;
    }
    return NULL;
}

void* loop_physics(void* arg)
{
    collider* playerCollider;
    collider temp = collider_createBoxCollider((vec3) { 0, 50, 0 }, (vec3) { 0.5f, 1.79f, 0.5f }, 0, 1, 0);
    physicsSystem_addCollider(&ps, temp);
    physicsSystem_processPending(&ps);//make sure that the collider is loaded into the physics system
    playerCollider = physicsSystem_getCollider(&ps, temp.id);


    float lastFrame = glfwGetTime();
    vec3 previousCumPosition = cum.position;
    while (69)
    {
        float currentTime = glfwGetTime();
        while (currentTime - lastFrame < PHYSICS_UPDATE)
        {
            physicsSystem_processPending(&ps);
            currentTime = glfwGetTime();
        }
        lastFrame = currentTime;

       
        //input
        input_update();
        event e;
        while ((e = event_queue_poll()).type != NONE)
            handle_event(e);

        double mouseX = 0, mouseY = 0;
        input_get_mouse_position(&mouseX, &mouseY);
        canvas_checkMouseInput(vaszon, mouseX, mouseY, input_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT));

        //player part
        pthread_mutex_lock(&mutex_cum);
        previousCumPosition = cum.position;
        cum.position = vec3_sum(playerCollider->position, (vec3) { 0, 0.69f, 0 });
        
        //keyboard
        vec3 velocity = (vec3){ 0,0,0 };
        vec3 forward = vec3_normalize(vec3_create2(cum.front.x, 0, cum.front.z));
        if (input_is_key_down(GLFW_KEY_W))
            velocity = vec3_sum(velocity, vec3_scale(forward, cum.move_speed));
        if (input_is_key_down(GLFW_KEY_S))
            velocity = vec3_sum(velocity, vec3_scale(forward, -cum.move_speed));
        if (input_is_key_down(GLFW_KEY_A))
            velocity = vec3_sum(velocity, vec3_scale(cum.right, -cum.move_speed));
        if (input_is_key_down(GLFW_KEY_D))
            velocity = vec3_sum(velocity, vec3_scale(cum.right, cum.move_speed));
        if (input_is_key_down(GLFW_KEY_LEFT_SHIFT))
            velocity = vec3_sum(velocity, (vec3) { 0, -cum.move_speed, 0 });
        if (input_is_key_down(GLFW_KEY_SPACE))
            velocity = vec3_sum(velocity, (vec3) { 0, cum.move_speed, 0 });

        playerCollider->velocity = velocity;

        //mouse buttons
        if (input_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            if (raycastFound != 0)
            {
                chunkManager_changeBlock(&cm, raycastChunkX, raycastChunkY, raycastChunkZ, raycastBlockX, raycastBlockY, raycastBlockZ, BLOCK_AIR);
                chunkManager_reloadChunk(&cm, &mutex_cm, raycastChunkX, raycastChunkY, raycastChunkZ);

                if (raycastBlockX == 0)
                    chunkManager_reloadChunk(&cm, &mutex_cm, raycastChunkX - 1, raycastChunkY, raycastChunkZ);
                else if (raycastBlockX == CHUNK_WIDTH - 1)
                    chunkManager_reloadChunk(&cm, &mutex_cm, raycastChunkX + 1, raycastChunkY, raycastChunkZ);
                if (raycastBlockY == 0)
                    chunkManager_reloadChunk(&cm, &mutex_cm, raycastChunkX, raycastChunkY - 1, raycastChunkZ);
                else if (raycastBlockY == CHUNK_HEIGHT - 1)
                    chunkManager_reloadChunk(&cm, &mutex_cm, raycastChunkX, raycastChunkY + 1, raycastChunkZ);
                if (raycastBlockZ == 0)
                    chunkManager_reloadChunk(&cm, &mutex_cm, raycastChunkX, raycastChunkY, raycastChunkZ - 1);
                else if (raycastBlockZ == CHUNK_WIDTH - 1)
                    chunkManager_reloadChunk(&cm, &mutex_cm, raycastChunkX, raycastChunkY, raycastChunkZ + 1);
            }
        }
        if (input_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT))
        {
            if (raycastFound != 0)
            {
                int tempRaycastChunk[3] = { raycastChunkX, raycastChunkY, raycastChunkZ };
                int tempRaycastBlock[3] = { raycastBlockX, raycastBlockY, raycastBlockZ };
                int tempChunkBounds[3] = { CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH };
                for (int i = 0; i < 3; i++)
                {
                    if ((&rh.normal.x)[i] > 0.01f) 
                    {
                        tempRaycastBlock[i]++;
                        if (tempRaycastBlock[i] == tempChunkBounds[i])
                        {
                            tempRaycastBlock[i] = 0;
                            tempRaycastChunk[i]++;
                        }
                    }
                    else if ((&rh.normal.x)[i] < -0.01f)
                    {
                        tempRaycastBlock[i]--;
                        if (tempRaycastBlock[i] == -1)
                        {
                            tempRaycastBlock[i] = tempChunkBounds[i]-1;
                            tempRaycastChunk[i]--;
                        }
                    }
                }
                chunkManager_changeBlock(&cm, tempRaycastChunk[0], tempRaycastChunk[1], tempRaycastChunk[2], tempRaycastBlock[0], tempRaycastBlock[1], tempRaycastBlock[2], BLOCK_SUS);
                chunkManager_reloadChunk(&cm, &mutex_cm, tempRaycastChunk[0], tempRaycastChunk[1], tempRaycastChunk[2]);
            }
        }

        //mouse movement
        double dx, dy;
        input_get_mouse_delta(&dx, &dy);
        dx *= cum.mouse_sensitivity;
        dy *= cum.mouse_sensitivity;
        cum.yaw -= dx;
        if (cum.yaw > 180.0f)
            cum.yaw -= 360.0f;
        if (cum.yaw < -180.0f)
            cum.yaw += 360.0f;

        cum.pitch -= dy;
        if (cum.pitch > 89.0f)
            cum.pitch = 89.0f;
        if (cum.pitch < -89.0f)
            cum.pitch = -89.0f;

        camera_updateVectors(&cum);
        pthread_mutex_unlock(&mutex_cum);
        //player part done


        //physics update
        double time = glfwGetTime();
        physicsSystem_resetCollisions(&ps);
        for (int i = 0; i < PHYSICS_STEPS_PER_UPDATE; i++)
        {
            physicsSystem_update(&ps, PHYSICS_UPDATE/PHYSICS_STEPS_PER_UPDATE);
        }
        rh.position = (vec3){ 0,100000,0 };
        raycastFound=physicsSystem_raycast(&ps, cum.position, cum.front, 6, 0.01f, &rh);

        rh.position=vec3_sum(rh.position, vec3_scale(rh.normal, -0.02f));
        rh.position = vec3_sum(rh.position, vec3_scale(cum.front, 0.02f));
        chunk_getChunkFromPos(rh.position, &raycastChunkX, &raycastChunkY, &raycastChunkZ);
        raycastBlockX = (int)(rh.position.x - CHUNK_WIDTH * raycastChunkX);
        raycastBlockY = (int)(rh.position.y - CHUNK_HEIGHT * raycastChunkY);
        raycastBlockZ = (int)(rh.position.z - CHUNK_WIDTH * raycastChunkZ);
        //printf("%.2f %.2f %.2f | %d %d %d\n", rh.position.x, rh.position.y, rh.position.z, CHUNK_WIDTH* raycastChunkX + raycastBlockX, CHUNK_HEIGHT* raycastChunkY + raycastBlockY, CHUNK_WIDTH* raycastChunkZ + raycastBlockZ);
        
        //player animation
        pthread_mutex_lock(&mutex_pm);
        pm.position = (vec3){ cum.position.x, cum.position.y - 1.6f, cum.position.z };
        pm.rotX = cum.pitch;
        pm.rotY = cum.yaw;
        pm.rotHeadX = cum.pitch;
        playerMesh_animate(&pm, (vec3_sqrMagnitude((vec3) { previousCumPosition.x - cum.position.x, 0, previousCumPosition.z - cum.position.z }) > 0.00001f) ? PLAYER_MESH_ANIMATION_WALK : PLAYER_MESH_ANIMATION_IDLE, PHYSICS_UPDATE);
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
            shouldExit2 = exitStatus;
            pthread_mutex_unlock(&mutex_exit);

            //query shouldPoll
            pthread_mutex_lock(&mutex_input);
            shouldPoll2 = shouldPoll;
            pthread_mutex_unlock(&mutex_input);

            if (shouldExit2!=EXIT_STATUS_REMAIN || !shouldPoll2)
                break;
        }
        if (shouldExit2)
            break;
    }


    return NULL;
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
            exitStatus = EXIT_STATUS_RETURN_TO_MENU;
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
        NULL);//"../assets/shaders/renderer/deferred_geometry/shader_deferred_geometry.gag"

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
    glUniform1f(glGetUniformLocation(waterShader.id, "shadowStart"), 140);
    glUniform1f(glGetUniformLocation(waterShader.id, "shadowEnd"), 150);
    glUniform1f(glGetUniformLocation(waterShader.id, "shadowHelper"), 1.0f / (150 - 140));

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
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogHelper"), 1.0f / (160 - 150));
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "shadowStart"), 90);
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "shadowEnd"), 100);
    glUniform1f(glGetUniformLocation(lightingPassShader.id, "shadowHelper"), 1.0f / (100 - 90));

    forwardPassShader = shader_import(
        "../assets/shaders/renderer/forward/shader_forward.vag",
        "../assets/shaders/renderer/forward/shader_forward.fag",
        NULL
    );

    bloomFilterShader = shader_import(
        "../assets/shaders/renderer/bloom/shader_bloom.vag",
        "../assets/shaders/renderer/bloom/shader_bloom_filter.fag",
        NULL
    );
    glUseProgram(bloomFilterShader.id);
    glUniform1i(glGetUniformLocation(bloomFilterShader.id, "tex"), 0);
    glUniform1f(glGetUniformLocation(bloomFilterShader.id, "threshold"), 5);

    bloomDownsampleShader = shader_import(
        "../assets/shaders/renderer/bloom/shader_bloom.vag",
        "../assets/shaders/renderer/bloom/shader_bloom_downsample.fag",
        NULL
    );
    glUseProgram(bloomDownsampleShader.id);
    glUniform1i(glGetUniformLocation(bloomDownsampleShader.id, "tex"), 0);

    finalPassShader = shader_import(
        "../assets/shaders/renderer/final_pass/shader_final_pass.vag",
        "../assets/shaders/renderer/final_pass/shader_final_pass.fag",
        NULL
    );
    glUseProgram(finalPassShader.id);
    glUniform1i(glGetUniformLocation(finalPassShader.id, "tex"), 0);
    char buffer[11];
    for (int i = 1; i < RENDERER_KAWASAKI_FBO_COUNT; i++)//1tol indul, mert az elso bloombuffer nincs mintavetelezve
    {
        sprintf(buffer, "bloom%d", i);
        glUniform1i(glGetUniformLocation(finalPassShader.id, buffer), i);
    }
    glUniform1f(glGetUniformLocation(finalPassShader.id, "exposure"), 0.2);

    fxaaShader = shader_import(
        "../assets/shaders/renderer/fxaa/shader_fxaa.vag",
        "../assets/shaders/renderer/fxaa/shader_fxaa.fag",
        NULL
    );
    glUseProgram(fxaaShader.id);
    glUniform1i(glGetUniformLocation(fxaaShader.id, "tex"), 0);
    glUniform2f(glGetUniformLocation(fxaaShader.id, "onePerResolution"), 1.0f / RENDERER_WIDTH, 1.0f / RENDERER_HEIGHT);

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
    glBindBuffer(GL_ARRAY_BUFFER, rectangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
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
    glUniform3f(glGetUniformLocation(skyboxShader.id, "sunColour"), 1, 1, 1);
    glUniform3f(glGetUniformLocation(skyboxShader.id, "skyColour0"), 0, 0.8f, 1.0f);
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
    pm = playerMesh_create();
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
    shader_delete(&bloomFilterShader);
    shader_delete(&bloomDownsampleShader);
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

void init_canvas()
{
    vaszon = canvas_create(window_getWidth(), window_getHeight(), "../assets/fonts/Monocraft.ttf");

    //left side
    vaszon_fps = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 15, 0, 0, 0, 24);
    vaszon_pos = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 40, 0, 0, 0, 24);
    vaszon_rot = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 65, 0, 0, 0, 24);

    vaszon_render_distance = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 105, 0, 0, 0, 24);
    vaszon_chunk_updates = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 130, 0, 0, 0, 24);
    vaszon_chunks_loaded = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 155, 0, 0, 0, 24);
    vaszon_chunks_rendered = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 180, 0, 0, 0, 24);

    //right side
    const char* vendor = glGetString(GL_VENDOR);
    const char* renderer = glGetString(GL_RENDERER);

    canvas_addText(vaszon, vendor, CANVAS_ALIGN_RIGHT, CANVAS_ALIGN_TOP, 15, 10, 0, 0, 0, 24);
    canvas_addText(vaszon, renderer, CANVAS_ALIGN_RIGHT, CANVAS_ALIGN_TOP, 15, 35, 0, 0, 0, 24);

    //cursor (should be replaced in a different canvas later)
    int temp;
    temp = canvas_addImage(vaszon, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, 0, 16, 16, textureHandler_getTexture(TEXTURE_ATLAS_UI));
    canvas_setImageUV(vaszon, temp, 0.0f, 0.9f, 0.1f, 0.1f);
}

void end_canvas()
{
    canvas_destroy(vaszon);
}


void window_size_callback(GLFWwindow* window, int width, int height)
{
    if (event_queue_back().type != WINDOW_RESIZE)
        event_queue_push((event) { .type = WINDOW_RESIZE, .data.window_resize = { width, height } });
    else
        event_queue_swap_back((event) { .type = WINDOW_RESIZE, .data.window_resize = { width, height } });
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
        event_queue_push((event) { .type = KEY_PRESSED, .data.key_pressed = { key } });
    else if (action == GLFW_RELEASE)
        event_queue_push((event) { .type = KEY_RELEASED, .data.key_released = { key } });
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS)
        event_queue_push((event) { .type = MOUSE_BUTTON_PRESSED, .data.mouse_button_pressed = { button } });
    else if (action == GLFW_RELEASE)
        event_queue_push((event) { .type = MOUSE_BUTTON_RELEASED, .data.mouse_button_released = { button } });
}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    event_queue_push((event) { .type = MOUSE_MOVED, .data.mouse_moved = { xpos, ypos } });
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    event_queue_push((event) { .type = MOUSE_SCROLLED, .data.mouse_scrolled = { xoffset, yoffset } });
}

void init_cube()
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
}

void end_cube()
{
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
}

double lerp(double a, double b, double f)
{
    return a + f * (b - a);
}