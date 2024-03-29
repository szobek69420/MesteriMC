#define _CRT_SECURE_NO_WARNINGS

#include "game.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

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
#include "../../renderer/hand/hand_renderer.h"
#include "../../renderer/stars/stars.h"

#include "../../ui/font_handler/font_handler.h"
#include "../../ui/text/text_renderer.h"
#include "../../ui/canvas/canvas.h"

#include "../../post_processing/lens_flare/flare.h"

#include "../../glm2/mat4.h"
#include "../../glm2/vec3.h"
#include "../../glm2/mat3.h"

#include "../../physics/collider/collider.h"
#include "../../physics/collider_group/collider_group.h"
#include "../../physics/physics_system/physics_system.h"

#include "../../utils/list.h"
#include "../../utils/vector.h"
#include "../../utils/lista.h"

#include "../../settings/settings.h"

#include "../../audio/audio.h"


float CLIP_NEAR = 0.1f;
float CLIP_FAR = 250.0f;

const float MOVEMENT_SPEED_BASED = 4;
const float GRAVITY_BASED = -25.0f;
const float JUMP_STRENGTH_BASED = 10;
const float FOV_BASED = 90;
float MOVEMENT_SPEED = 4;
float GRAVITY = -25.0;
float JUMP_STRENGTH = 10;
float FOV = 90;

const float LENGTH_OF_DAY_IN_SECONDS = 1000;
float TIME_OF_DAY = 0.0f;//napkelte a 0, [0;1)-ben van az �rt�ke

#define PHYSICS_UPDATE 0.02f
#define PHYSICS_STEPS_PER_UPDATE 50
#define GENERATION_UPDATE 0.001f
#define CHUNK_UPDATES_PER_GENERATION_UPDATE 4
#define CHUNK_MESH_UPDATES_PER_FRAME 20
#define CHUNK_UPDATE_BURST_SIZE 2

#define HOTBAR_SIZE 5

#define INVENTORY_ROWS 6
#define INVENTORY_COLUMNS 4

#define COMMAND_LINE_MAX_ARGUMENT_COUNT 20

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
int currentCursorMode, targetCursorMode;//glfwSetInputMode should only be called from the main thread, so these serve as a buffer until the main thread gets to change the input mode

enum gameState{
	GAME_INGAME, GAME_PAUSED, GAME_INVENTORY, GAME_COMMAND_LINE
};
int currentGameState;//is the game paused or in game or idk
pthread_mutex_t mutex_gameState;

enum playerState {
	PLAYER_MORTAL=1, PLAYER_IMMORTAL=2
};
int currentPlayerState = PLAYER_MORTAL;

enum commandType {
	COMMAND_NOT_COMMAND, //there is only a plain text without the command sign in the input
	COMMAND_UNKNOWN, //the input is in command format, but the command does not exit
	COMMAND_GAMEMODE, 
	COMMAND_SPEED, 
	COMMAND_JUMP, 
	COMMAND_GRAVITY,
	COMMAND_TELEPORT,
	COMMAND_TIME
};

pthread_mutex_t mutex_window;//for window calls (like window_getHeight())

physicsSystem* ps;
float physicsUpdateTime = 0;
raycastHit rh;
int raycastChunkX, raycastChunkY, raycastChunkZ;
int raycastBlockX, raycastBlockY, raycastBlockZ;
int raycastFound = 0;
vec3 lastRaycastHit;

chunkManager cm;
pthread_mutex_t mutex_cm;
int chunkUpdatesInLastSecond = 0;

camera cum;
pthread_mutex_t mutex_cum;
float currentFov = 90;
int isCameraSubmerged = 0;

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
shader finalPassNonHdrShader;
shader skyboxShader; mesh skyboxMesh;

canvas* vaszonPause;
pthread_mutex_t mutex_vaszonPause;

canvas* vaszonIngame;
pthread_mutex_t mutex_vaszonIngame;
int vaszonIngame_hotbarSlotFrames[HOTBAR_SIZE];
int vaszonIngame_hotbarSlotBlockMeshes[HOTBAR_SIZE];
int vaszonIngame_selectedBlockText;

canvas* vaszonInventory;
pthread_mutex_t mutex_vaszonInventory;
int vaszonInventory_inventorySlotFrames[INVENTORY_COLUMNS * INVENTORY_ROWS];
int vaszonInventory_inventorySlotBlockMeshes[INVENTORY_COLUMNS * INVENTORY_ROWS];
int vaszonInventory_inventoryHotbarSlotFrames[HOTBAR_SIZE];
int vaszonInventory_inventoryHotbarSlotBlockMeshes[HOTBAR_SIZE];
int vaszonInventory_dragged;

canvas* vaszonCommand;
pthread_mutex_t mutex_vaszonCommand;
const int COMMAND_LINE_FONT_SIZE = 21;
const int COMMAND_LINE_HISTORY_LENGTH = 20;
char currentCommand[100];
float currentCommandWidth = 0;
char commandHistory[20][100]; int indexOfMostRecentCommand = 0;//ha pl az index 3, akkor a 3. elemtol kezdi a kiirast es ha eleri a 19-iket, akkor utana visszaugrik 0-ra es onnan folytatja
int vaszonCommand_commandText;
int vaszonCommand_commandHistory[20];
int vaszonCommand_commandCursor;


canvas* vaszon;//debug screen
pthread_mutex_t mutex_vaszon;
int vaszon_fps;
int vaszon_pos;
int vaszon_rot;
int vaszon_render_distance;
int vaszon_chunk_updates;
int vaszon_chunks_loaded;
int vaszon_chunks_pending_updates;
int vaszon_chunks_rendered;
int vaszon_physics_update_time;
int vaszon_physics_simulated_colliders;
int vaszon_physics_loaded_groups;
int vaszon_physics_raycast_hit;


unsigned int rectangleVAO;//a deferred resz hasznalja a kepernyo atrajzolasahoz
unsigned int rectangleVBO;


int hotbarContent[HOTBAR_SIZE];
int hotbarSlotSelected = 0;

int inventoryContent[INVENTORY_ROWS * INVENTORY_COLUMNS];
//the ids of the slots whose content should be swapped the next graphics(!!!) frame
//here (and in the callback functions) the inventory hotbar slots will be represented with the id of 100+slotID
int inventorySlotContentSwap[2] = { -1, -1 };
int draggedInventorySlot = -1;
int inventoryDropTarget = -1;//where to drop the dragged item


vector* lights;
light sunTzu;
light moonTzu;

light_renderer lightRenderer;
vec3 sunDirection;

stars* nightSky;

flare* lensFlare;

handRenderer* hr;

//function prototypes
GLFWwindow* init_window(const char* name, int width, int height);
void handle_event(event e);

void changeGameState(int inBounds, int gs);

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

//ui callbacks
void quitGame(int inBounds, void* nichts);

void inventorySlotPress(int inventorySlotID);
void inventorySlotRelease(int inBounds, int inventorySlotID);
void inventorySlotExit(int inventorySlotID);
void inventorySlotEnter(int inventorySlotID);

//command line functions
void convertCommandLine(const char* command, int* commandType, int* argumentCount, int* arguments);
void updateCommandHistory(const char* newCommand);

//glfw callbacks
void window_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void command_line_character_callback(GLFWwindow* window, unsigned int codepoint);

void game(void* w, int* currentStage)
{
	//callbacks
	window = (GLFWwindow*)w;
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	currentCursorMode = GLFW_CURSOR_DISABLED;
	targetCursorMode = GLFW_CURSOR_DISABLED;

	currentGameState = GAME_INGAME;

	currentPlayerState = PLAYER_IMMORTAL;

	TIME_OF_DAY = 0;
	sunDirection = (vec3){ 0.0f, 0.0f, 1.0f };

	int tempHotbarContent[HOTBAR_SIZE]={ BLOCK_SUS ,BLOCK_DIRT ,BLOCK_GRASS ,BLOCK_OAK_LOG ,BLOCK_OAK_LEAVES };
	memcpy(hotbarContent, tempHotbarContent, sizeof(hotbarContent));
	int tempInventoryContent[INVENTORY_COLUMNS * INVENTORY_ROWS] = {
		BLOCK_GRASS, BLOCK_DIRT, BLOCK_STONE, BLOCK_AIR,
		BLOCK_OAK_LOG, BLOCK_OAK_LEAVES, BLOCK_AIR, BLOCK_AIR,
		BLOCK_SUS, BLOCK_BORSOD, BLOCK_AIR, BLOCK_AIR,
		BLOCK_AIR, BLOCK_AIR, BLOCK_AIR, BLOCK_AIR,
		BLOCK_AIR, BLOCK_AIR, BLOCK_AIR, BLOCK_AIR,
		BLOCK_AIR, BLOCK_AIR, BLOCK_AIR, BLOCK_AIR,
	};
	memcpy(inventoryContent, tempInventoryContent, sizeof(inventoryContent));

	textureHandler_importTextures(TEXTURE_IN_GAME);
	
	event_queue_init();
	input_init();

	audio_init(AUDIO_INGAME);

	fontHandler_init();
	init_canvas();

	CLIP_FAR = settings_getInt(SETTINGS_RENDER_DISTANCE) * CHUNK_WIDTH + 50;
	cum = camera_create(vec3_create2(0, 50, 0), vec3_create2(0, 1, 0), 4, 0.2);
	camera_setProjection(&cum, currentFov, window_getAspect(), CLIP_NEAR, CLIP_FAR);
	cum.fov = FOV_BASED;

	ps = physicsSystem_create();

	cm = chunkManager_create(69, settings_getInt(SETTINGS_RENDER_DISTANCE), ps);
	chunk_resetGenerationInfo();
	init_renderer();

	init_cube();//egyszer majd ki kene szedni

	blockSelection_init();
	blockSelection_setColour(CANVAS_COLOUR_PRIMARY_1);
	hr = handRenderer_create();
	handRenderer_setBlock(hr, hotbarContent[hotbarSlotSelected]);

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
	pthread_mutex_init(&mutex_vaszonPause, NULL);
	pthread_mutex_init(&mutex_vaszonIngame, NULL);
	pthread_mutex_init(&mutex_vaszonInventory, NULL);
	pthread_mutex_init(&mutex_vaszonCommand, NULL);
	pthread_mutex_init(&mutex_gameState, NULL);


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
		if (currentCursorMode != targetCursorMode)
		{
			currentCursorMode = targetCursorMode;
			glfwSetInputMode(window, GLFW_CURSOR, targetCursorMode);
		}
		pthread_mutex_unlock(&mutex_input);

		//check if it should quit
		int shouldExit2 = 0;
		pthread_mutex_lock(&mutex_exit);
		if (glfwWindowShouldClose(window))
			exitStatus = EXIT_STATUS_CLOSE_PROGRAM;
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
	pthread_mutex_destroy(&mutex_vaszonPause);
	pthread_mutex_destroy(&mutex_vaszonIngame);
	pthread_mutex_destroy(&mutex_vaszonInventory);
	pthread_mutex_destroy(&mutex_vaszonCommand);
	pthread_mutex_destroy(&mutex_gameState);


	glfwMakeContextCurrent(window);

	chunkManager_destroy(&cm);
	physicsSystem_destroy(ps);
	textureHandler_destroyTextures(TEXTURE_IN_GAME);
	end_canvas();
	fontHandler_close();
	end_renderer();

	audio_destroy();

	end_cube();

	blockSelection_close();
	handRenderer_destroy(hr);

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

	float selectedBlockAppearance = 0;
	int previousHotbarSlot = hotbarSlotSelected;
	int previousGameState = GAME_INGAME;
	int previousDraggedInventorySlot = -1;

	int loadedChunks = 0;
	int pendingMeshUpdates, pendingGenerationUpdates;
	
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
		pendingGenerationUpdates = cm.pendingUpdates.size;
		pendingMeshUpdates = cm.pendingMeshUpdates.size;
		for (int i = 0; i < CHUNK_MESH_UPDATES_PER_FRAME&&i<pendingMeshUpdates; i++)
		{
			chunkManager_updateMesh(&cm);
		}

		loadedChunks = cm.loadedChunks.size;
		pthread_mutex_unlock(&mutex_cm);

		//render------------------------------------------
		//matrices and variables
		mat4 view = camera_getViewMatrix(&cum_render);
		mat4 projection = cum_render.projection_matrix;
		mat4 pv = mat4_multiply(projection, view);
		mat4 projectionInverse = mat4_inverse(projection);
		mat3 viewNormal = mat3_createFromMat(view);

		int isDayTime = 0;
		if (vec3_dot(vec3_normalize(sunDirection), (vec3) { 0, 1, 0 }) > 0)
			isDayTime = 1;
		float sunShadowIntensity = powf(fabs(vec3_dot(vec3_normalize(sunDirection), (vec3) { 0, 1, 0 })), 0.5f);
		float moonShadowIntensity = powf(fabs(vec3_dot(vec3_normalize(sunDirection), (vec3) { 0, -1, 0 })), 0.5f);
		mat4 shadowViewProjection;
		if (isDayTime)
		{
			shadowViewProjection = mat4_multiply(
				mat4_ortho(-250, 250, -250, 250, 1, 200),
				mat4_lookAt(
					vec3_sum(cum_render.position, vec3_create2(sunDirection.x * 100, sunDirection.y * 100, sunDirection.z * 100)),
					vec3_create2(-1 * sunDirection.x, -1 * sunDirection.y, -1 * sunDirection.z),
					vec3_create2(0, 1, 0)
				)
			);
		}
		else
		{
			shadowViewProjection = mat4_multiply(
				mat4_ortho(-250, 250, -250, 250, 1, 200),
				mat4_lookAt(
					vec3_sum(cum_render.position, vec3_create2(sunDirection.x * -100, sunDirection.y * -100, sunDirection.z * -100)),
					sunDirection,
					vec3_create2(0, 1, 0)
				)
			);
		}
		mat4 shadowLightMatrix = mat4_multiply(shadowViewProjection, mat4_inverse(view));//from the camera's view space to the suns projection space
		
		//fog distance
		int fogEnd = settings_getInt(SETTINGS_RENDER_DISTANCE) * CHUNK_WIDTH - 10;

		//shadow
		if (settings_getInt(SETTINGS_SHADOWS) != 0)
		{
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
		}
		

		//prepare gbuffer fbo
		glViewport(0, 0, RENDERER_WIDTH, RENDERER_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, rendor.gBuffer.id);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//geometry pass ------------------------------------------------------------------------------------------
		glUseProgram(geometryPassShader.id);
		glEnable(GL_DEPTH_TEST);

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

		do {//hand
			if (previousHotbarSlot != hotbarSlotSelected)
				handRenderer_setBlock(hr, hotbarContent[hotbarSlotSelected]);

			if (hotbarContent[hotbarSlotSelected] != BLOCK_AIR)
			{
				mat4 mamogus = mat4_create(1);
				mat3 mamogus2;
				vec3 handPos = cum_render.position;
				handPos = vec3_sum(handPos, vec3_scale(cum_render.front, 0.3f));
				handPos = vec3_sum(handPos, vec3_scale(cum_render.right, 0.3f));
				handPos = vec3_sum(handPos, vec3_scale(cum_render.up, -0.3f));

				mamogus = mat4_translate(mamogus, handPos);
				mamogus = mat4_rotate(mamogus, (vec3) { 0, 1, 0 }, cum_render.yaw + 10);
				mamogus = mat4_rotate(mamogus, (vec3) { 1, 0, 0 }, cum_render.pitch);
				mamogus = mat4_scale(mamogus, (vec3) { 0.2f, 0.2f, 0.2f });

				mamogus2 = mat3_createFromMat(mat4_transpose(mat4_inverse(mamogus)));

				handRenderer_setMatrices(hr, &projection, &view, &viewNormal);
				glDepthFunc(GL_ALWAYS);
				handRenderer_renderBlock(hr, &mamogus, &mamogus2);
				glDepthFunc(GL_LESS);
			}
		} while (0);
		
		//copy depth buffer
		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, rendor.gBuffer.id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rendor.endBuffer.id);
		glBlitFramebuffer(0, 0, RENDERER_WIDTH, RENDERER_HEIGHT, 0, 0, RENDERER_WIDTH, RENDERER_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		

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
		
		if (settings_getInt(SETTINGS_SHADOWS) != 0)
		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, rendor.shadowBuffer.depthBuffer);
		}

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, rendor.gBuffer.depthBuffer);

		glUseProgram(lightingPassShader.id);
		glUniformMatrix4fv(glGetUniformLocation(lightingPassShader.id, "view"), 1, GL_FALSE, view.data);
		glUniformMatrix4fv(glGetUniformLocation(lightingPassShader.id, "projection"), 1, GL_FALSE, projection.data);
		glUniformMatrix4fv(glGetUniformLocation(lightingPassShader.id, "projection_inverse"), 1, GL_FALSE, projectionInverse.data);

		if (isCameraSubmerged != 0)//ha vizben van, akkor kevesebb a latotavolsag
		{
			glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogStart"), 1);
			glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogEnd"), 11);
		}
		else
		{
			glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogStart"), fogEnd - 10);
			glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogEnd"), fogEnd);
		}

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

		//directional (sun and moon)
		do {//sun
			sunTzu.position = sunDirection;//move sun according to day-night cycle
			sunTzu.colour = (vec3){ 1, 0.97, 0.4 };

			float intensity= 10* (0.9f * vec3_dot(vec3_normalize(sunDirection), (vec3) { 0, 1, 0 }) + 0.1f);
			if (intensity < 0)
			{
				sunTzu.attenuation.x = 0;
				break;
			}

			sunTzu.attenuation.x = intensity;

			if (settings_getInt(SETTINGS_SHADOWS) != 0&&sunShadowIntensity>0)
			{
				glUniformMatrix4fv(glGetUniformLocation(lightingPassShader.id, "shadow_lightMatrix"), 1, GL_FALSE, shadowLightMatrix.data);
				glUniform1i(glGetUniformLocation(lightingPassShader.id, "shadowOn"), 69);
				glUniform1f(glGetUniformLocation(lightingPassShader.id, "shadowStrength"), sunShadowIntensity);
			}
			else
				glUniform1i(glGetUniformLocation(lightingPassShader.id, "shadowOn"), 0);


			bufferData = (float*)malloc(LIGHT_SIZE_IN_VBO);
			memcpy(bufferData, &(sunTzu.position.x), LIGHT_SIZE_IN_VBO);
			light_fillRenderer(&lightRenderer, bufferData, 1);
			light_render(&lightRenderer, 0);
			free(bufferData);
		} while (0);
		do {//moon
			moonTzu.position = vec3_scale(sunDirection,-1);//move moon according to day-night cycle
			moonTzu.colour = (vec3){ 1,1,1 };

			float intensity = (0.9f * vec3_dot(vec3_normalize(sunDirection), (vec3) { 0, -1, 0 }) + 0.1f);
			if (intensity < 0)
			{
				moonTzu.attenuation.x = 0;
				break;
			}

			moonTzu.attenuation.x = intensity;

			if (settings_getInt(SETTINGS_SHADOWS) != 0 && moonShadowIntensity > 0)
			{
				glUniformMatrix4fv(glGetUniformLocation(lightingPassShader.id, "shadow_lightMatrix"), 1, GL_FALSE, shadowLightMatrix.data);
				glUniform1i(glGetUniformLocation(lightingPassShader.id, "shadowOn"), 69);
				glUniform1f(glGetUniformLocation(lightingPassShader.id, "shadowStrength"), moonShadowIntensity);
			}
			else
				glUniform1i(glGetUniformLocation(lightingPassShader.id, "shadowOn"), 0);



			bufferData = (float*)malloc(LIGHT_SIZE_IN_VBO);
			memcpy(bufferData, &(moonTzu.position.x), LIGHT_SIZE_IN_VBO);
			light_fillRenderer(&lightRenderer, bufferData, 1);
			light_render(&lightRenderer, 0);
			free(bufferData);
		} while (0);

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
			vec3 sSize = vec3_create2(1.000f, 1.000f, 1.000f);
			blockSelection_render(sPos, sSize, &pv);
		} while (0);
		
		//walter
		glUseProgram(waterShader.id);

		glUniform1f(glGetUniformLocation(waterShader.id, "uvOffset"), 0.03f * glfwGetTime());

		glUniform3f(glGetUniformLocation(waterShader.id, "waterColourDeep"), 0, 0.0627f, 0.8f);
		glUniform3f(glGetUniformLocation(waterShader.id, "waterColourShallow"), 0, 0.8627f, 0.8941f);

		glUniformMatrix4fv(glGetUniformLocation(waterShader.id, "projectionInverse"), 1, GL_FALSE, projectionInverse.data);

		if (settings_getInt(SETTINGS_SHADOWS)!=0)
		{
			glUniformMatrix4fv(glGetUniformLocation(waterShader.id, "shadow_lightMatrix"), 1, GL_FALSE, shadowLightMatrix.data);
			glUniform1i(glGetUniformLocation(waterShader.id, "shadowOn"), 1);
			glUniform1f(glGetUniformLocation(waterShader.id, "shadowStrength"), isDayTime != 0 ? sunShadowIntensity : moonShadowIntensity);
		}
		else
			glUniform1i(glGetUniformLocation(waterShader.id, "shadowOn"), 0);

		if (isCameraSubmerged != 0)
		{
			glUniform1f(glGetUniformLocation(waterShader.id, "fogStart"), 1);
			glUniform1f(glGetUniformLocation(waterShader.id, "fogEnd"), 11);
		}
		else
		{
			glUniform1f(glGetUniformLocation(waterShader.id, "fogStart"), fogEnd - 10);
			glUniform1f(glGetUniformLocation(waterShader.id, "fogEnd"), fogEnd);
		}

		//sun and moon (their parameters have been set in the deferred pass)
		glUniform3f(glGetUniformLocation(waterShader.id, "sun.position"), sunTzu.position.x, sunTzu.position.y, sunTzu.position.z);
		glUniform3f(glGetUniformLocation(waterShader.id, "sun.colour"), sunTzu.colour.x, sunTzu.colour.y, sunTzu.colour.z);
		glUniform3f(glGetUniformLocation(waterShader.id, "sun.attenuation"), sunTzu.attenuation.x, sunTzu.attenuation.y, sunTzu.attenuation.z);

		glUniform3f(glGetUniformLocation(waterShader.id, "moon.position"), moonTzu.position.x, moonTzu.position.y, moonTzu.position.z);
		glUniform3f(glGetUniformLocation(waterShader.id, "moon.colour"), moonTzu.colour.x, moonTzu.colour.y, moonTzu.colour.z);
		glUniform3f(glGetUniformLocation(waterShader.id, "moon.attenuation"), moonTzu.attenuation.x, moonTzu.attenuation.y, moonTzu.attenuation.z);


		glUniform3f(glGetUniformLocation(waterShader.id, "cameraPos"), cum_render.position.x, cum_render.position.y, cum_render.position.z);
		glUniformMatrix4fv(glGetUniformLocation(waterShader.id, "view"), 1, GL_FALSE, view.data);
		glUniformMatrix4fv(glGetUniformLocation(waterShader.id, "projection"), 1, GL_FALSE, projection.data);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_WATER_NORMAL));

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_WATER_DUDV));

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, rendor.gBuffer.depthBuffer);

		if (settings_getInt(SETTINGS_SHADOWS) != 0)
		{
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, rendor.shadowBuffer.depthBuffer);
		}

		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		pthread_mutex_lock(&mutex_cm);
		chunkManager_drawWalter(&cm, &waterShader, &cum, &projection);
		pthread_mutex_unlock(&mutex_cm);

		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);

		//get lens flare data
		flare_queryQueryResult(lensFlare);
		flare_query(lensFlare, &pv, cum_render.position, sunTzu.position, 1.0f / windowAspectXY);

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
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_SKY_GRADIENT_COLOUR_0));
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_SKY_GRADIENT_COLOUR_1));
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_SKY_GRADIENT_HORIZON));


		glUseProgram(skyboxShader.id);
		vec3 sunDirectionNormalized = vec3_normalize(sunDirection);
		glUniform1f(glGetUniformLocation(skyboxShader.id, "timeOfDay"), TIME_OF_DAY);
		glUniform3f(glGetUniformLocation(skyboxShader.id, "sunDirectionNormalized"), sunDirectionNormalized.x, sunDirectionNormalized.y, sunDirectionNormalized.z);
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.id, "pvm"), 1, GL_FALSE, mat4_multiply(pv, mat4_create2((float[]) { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, cum_render.position.x, cum_render.position.y, cum_render.position.z, 1 })).data);
		glBindVertexArray(skyboxMesh.vao);
		glDrawElements(GL_TRIANGLES, skyboxMesh.indexCount, GL_UNSIGNED_INT, 0);

		glFrontFace(GL_CCW);

		//stars
		if (isDayTime==0)
		{
			stars_setPlayerPosition(nightSky, cum_render.position);
			stars_setIntensity(nightSky, powf(moonShadowIntensity,2));
			stars_render(nightSky, pv);
		}

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
		do {
			float intensity = (0.9f * vec3_dot(vec3_normalize(sunDirection), (vec3) { 0, 1, 0 }) + 0.1f);
			if (intensity > 0&& isCameraSubmerged == 0)
			{
				flare_setStrength(lensFlare, intensity);
				flare_render(lensFlare, &pv, cum_render.position, sunTzu.position, 1.0f / windowAspectXY);
			}
		} while (0);

		//switch to default fbo ------------------------------------------------------------------------
		glViewport(0, 0, windowWidth, windowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);//default framebuffer

		//fxaa and other non hdr post processing things
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rendor.screenBuffer.colorBuffer);

		glUseProgram(finalPassNonHdrShader.id);
		glUniform1i(glGetUniformLocation(finalPassNonHdrShader.id, "isSubmerged"), isCameraSubmerged);
		glUniform1f(glGetUniformLocation(finalPassNonHdrShader.id, "currentTime"), (float)glfwGetTime());
		glBindVertexArray(rectangleVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//render 2d stuff (only text yet)
		if (shouldChangeSize)
		{
			pthread_mutex_lock(&mutex_vaszon);
			canvas_setSize(vaszon, windowWidth, windowHeight);
			pthread_mutex_unlock(&mutex_vaszon);

			pthread_mutex_lock(&mutex_vaszonPause);
			canvas_setSize(vaszonPause, windowWidth, windowHeight);
			pthread_mutex_unlock(&mutex_vaszonPause);

			pthread_mutex_lock(&mutex_vaszonIngame);
			canvas_setSize(vaszonIngame, windowWidth, windowHeight);
			pthread_mutex_unlock(&mutex_vaszonIngame);

			pthread_mutex_lock(&mutex_vaszonInventory);
			canvas_setSize(vaszonInventory, windowWidth, windowHeight);
			pthread_mutex_unlock(&mutex_vaszonInventory);

			pthread_mutex_lock(&mutex_vaszonCommand);
			canvas_setSize(vaszonCommand, windowWidth, windowHeight);
			pthread_mutex_unlock(&mutex_vaszonCommand);
		}

		static char buffer[50];

		double mouseX = 0, mouseY = 0;
		input_get_mouse_position(&mouseX, &mouseY);
	
		switch (currentGameState)
		{
			case GAME_INGAME:
				pthread_mutex_lock(&mutex_vaszon);
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

				sprintf(buffer, "pending: G%d, M%d", pendingGenerationUpdates, pendingMeshUpdates);
				canvas_setTextText(vaszon, vaszon_chunks_pending_updates, buffer);

				sprintf(buffer, "loaded chunks: %d", loadedChunks);
				canvas_setTextText(vaszon, vaszon_chunks_loaded, buffer);

				sprintf(buffer, "rendered chunks: %d", renderedChunks);
				canvas_setTextText(vaszon, vaszon_chunks_rendered, buffer);


				sprintf(buffer, "physics update: %.3f ms", 1000 * physicsUpdateTime);
				canvas_setTextText(vaszon, vaszon_physics_update_time, buffer);

				sprintf(buffer, "simulated colliders: %d", physicsSystem_getColliderCount(ps));
				canvas_setTextText(vaszon, vaszon_physics_simulated_colliders, buffer);

				sprintf(buffer, "collider groups: %d", physicsSystem_getColliderGroupCount(ps));
				canvas_setTextText(vaszon, vaszon_physics_loaded_groups, buffer);

				if (raycastFound != 0)
					sprintf(buffer, "raycast hit: %.2f, %.2f, %.2f", lastRaycastHit.x, lastRaycastHit.y, lastRaycastHit.z);
				else
					sprintf(buffer, "raycast hit: nein");
				canvas_setTextText(vaszon, vaszon_physics_raycast_hit, buffer);

				canvas_render(vaszon, mouseX, mouseY, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT));
				pthread_mutex_unlock(&mutex_vaszon);

				pthread_mutex_lock(&mutex_vaszonIngame);
				
				//hotbar
				if (previousHotbarSlot!=hotbarSlotSelected)
				{
					selectedBlockAppearance = glfwGetTime();
					canvas_setTextText(vaszonIngame, vaszonIngame_selectedBlockText, blocks_getBlockName(hotbarContent[hotbarSlotSelected]));
				}
				if(glfwGetTime()-selectedBlockAppearance>1.0f)
					canvas_setTextText(vaszonIngame, vaszonIngame_selectedBlockText, "");

				if (previousGameState == GAME_INVENTORY)//inventory should be reloaded because it could change
				{
					for (int i = 0; i < HOTBAR_SIZE; i++)
					{
						canvas_setBlockMeshBlock(vaszonIngame, vaszonIngame_hotbarSlotBlockMeshes[i], hotbarContent[i]);
					}
				}

				for (int i = 0; i < HOTBAR_SIZE; i++)
				{
					if (i == hotbarSlotSelected)
						canvas_setButtonBorderColour(vaszonIngame, vaszonIngame_hotbarSlotFrames[i], CANVAS_COLOUR_ACCENT_0);
					else
						canvas_setButtonBorderColour(vaszonIngame, vaszonIngame_hotbarSlotFrames[i], CANVAS_COLOUR_PRIMARY_1);
				}


				canvas_render(vaszonIngame, 0, 0, 0);
				pthread_mutex_unlock(&mutex_vaszonIngame);
				break;

			case GAME_PAUSED:
				pthread_mutex_lock(&mutex_vaszonPause);
				canvas_render(vaszonPause, mouseX, mouseY, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT));
				pthread_mutex_unlock(&mutex_vaszonPause);
				break;

			case GAME_INVENTORY:
				pthread_mutex_lock(&mutex_vaszonInventory);
				if (inventorySlotContentSwap[0] != -1)
				{
					int slot0, slot1, * content0, * content1;
					if (inventorySlotContentSwap[0] >= 100)
					{
						slot0 = vaszonInventory_inventoryHotbarSlotBlockMeshes[inventorySlotContentSwap[0] - 100];
						content0 = &(hotbarContent[inventorySlotContentSwap[0] - 100]);
					}
					else
					{
						slot0 = vaszonInventory_inventorySlotBlockMeshes[inventorySlotContentSwap[0]];
						content0 = &(inventoryContent[inventorySlotContentSwap[0]]);
					}

					if (inventorySlotContentSwap[1] >= 100)
					{
						slot1 = vaszonInventory_inventoryHotbarSlotBlockMeshes[inventorySlotContentSwap[1] - 100];
						content1 = &(hotbarContent[inventorySlotContentSwap[1] - 100]);
					}
					else
					{
						slot1 = vaszonInventory_inventorySlotBlockMeshes[inventorySlotContentSwap[1]];
						content1 = &(inventoryContent[inventorySlotContentSwap[1]]);
					}

					int temp = *content0;
					*content0 = *content1;
					*content1 = temp;

					canvas_setBlockMeshBlock(vaszonInventory, slot0, *content0);
					canvas_setBlockMeshBlock(vaszonInventory, slot1, *content1);

					inventorySlotContentSwap[0] = -1;
					inventorySlotContentSwap[1] = -1;
				}


				if (previousDraggedInventorySlot != draggedInventorySlot)
				{
					if (draggedInventorySlot >=100)//started draggin' a hotbar slot
						canvas_setBlockMeshBlock(vaszonInventory, vaszonInventory_dragged, hotbarContent[draggedInventorySlot - 100]);
					else if(draggedInventorySlot!=-1)//started draggin' an inventory slot
						canvas_setBlockMeshBlock(vaszonInventory, vaszonInventory_dragged, inventoryContent[draggedInventorySlot]);
					else//finished draggin'
					{
						canvas_setBlockMeshBlock(vaszonInventory, vaszonInventory_dragged, BLOCK_AIR);
						canvas_setComponentPosition(vaszonInventory, vaszonInventory_dragged, -100000, -100000);
					}
				}
				if (draggedInventorySlot != -1)
				{
					canvas_setComponentPosition(vaszonInventory, vaszonInventory_dragged, mouseX-0.5f * windowWidth, 0.5f * windowHeight - mouseY);
				}

				canvas_render(vaszonInventory, mouseX, mouseY, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT));
				pthread_mutex_unlock(&mutex_vaszonInventory);
				break;

			case GAME_COMMAND_LINE:
				pthread_mutex_lock(&mutex_vaszonCommand);
				canvas_setTextText(vaszonCommand, vaszonCommand_commandText, currentCommand);
				canvas_render(vaszonCommand, mouseX, mouseY, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT));
				pthread_mutex_unlock(&mutex_vaszonCommand);
				break;
		}

		previousHotbarSlot = hotbarSlotSelected;
		previousGameState = currentGameState;
		previousDraggedInventorySlot = draggedInventorySlot;

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

	int lastPlayerChunkX=0, lastPlayerChunkY=0, lastPlayerChunkZ=0;
	int shouldSearch=1;//true if there has been a chunk load/unload last update or the playerchunk has changed

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
		if (lastPlayerChunkX != chunkX || lastPlayerChunkY != chunkY || lastPlayerChunkZ != chunkZ)
			shouldSearch = 69;
		lastPlayerChunkX = chunkX; lastPlayerChunkY = chunkY; lastPlayerChunkZ = chunkZ;
		if (shouldSearch!=0)
		{
			pthread_mutex_lock(&mutex_cm);
			shouldSearch = chunkManager_searchForUpdates(&cm, chunkX, chunkY, chunkZ, 2);
			pthread_mutex_unlock(&mutex_cm);
		}

		for (int i = 0; i < CHUNK_UPDATES_PER_GENERATION_UPDATE; i++)
		{
			if (cm.pendingUpdates.size > 0)
			{
				chunkManager_update(&cm, &mutex_cm);
				chunkUpdates+=CHUNK_UPDATE_BURST_SIZE;
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
	int firstFrame = 69;

	collider* playerCollider;
	collider* cameraCollider;//a little collider for the camera to detect walter
	
	do {
		int playerColliderId=0, cameraColliderId=0;

		collider temp = collider_createBoxCollider((vec3) { 0, 50, 0 }, (vec3) { 0.5f, 1.79f, 0.5f }, 0, currentPlayerState==PLAYER_MORTAL, 0);
		physicsSystem_addCollider(ps, temp);
		playerColliderId = temp.id;

		temp = collider_createBoxCollider((vec3) { 0, 50, 0 }, (vec3) { 0.01f, 0.01f, 0.01f }, 0, 0, 0);
		physicsSystem_addCollider(ps, temp);
		cameraColliderId = temp.id;
		
		physicsSystem_processPendingAll(ps);

		playerCollider = physicsSystem_getCollider(ps, playerColliderId);
		cameraCollider = physicsSystem_getCollider(ps, cameraColliderId);

		if (playerCollider == NULL || cameraCollider == NULL)
			printf("amogus\n");

	} while (0);


	float lastFrame = glfwGetTime();
	vec3 previousCumPosition = cum.position;
	float lastAudioCleanup = 0;
	float nextCaveNoise = 0;
	while (69)
	{
		float currentTime = glfwGetTime();
		while (currentTime - lastFrame < PHYSICS_UPDATE)
		{
			physicsSystem_processPending(ps);
			currentTime = glfwGetTime();
		}
		lastFrame = currentTime;

		//audio
		if (currentTime - lastAudioCleanup > 5)
		{
			audio_cleanupUnused();
			lastAudioCleanup = currentTime;
		}

		//input
		input_update();
		event e;
		while ((e = event_queue_poll()).type != NONE)
			handle_event(e);

		double mouseX = 0, mouseY = 0;
		input_get_mouse_position(&mouseX, &mouseY);
		
		switch (currentGameState)
		{
		case GAME_INGAME:
			canvas_checkMouseInput(vaszon, mouseX, mouseY, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT),  input_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT));
			break;

		case GAME_PAUSED:
			canvas_checkMouseInput(vaszonPause, mouseX, mouseY, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT));
			break;

		case GAME_INVENTORY:
			canvas_checkMouseInput(vaszonInventory, mouseX, mouseY, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT));
			break;

		case GAME_COMMAND_LINE:
			canvas_checkMouseInput(vaszonCommand, mouseX, mouseY, input_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT), input_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT));
			break;
		}

		switch (currentGameState)
		{
			case GAME_INGAME:
				//player part
				if (firstFrame != 0)
				{
					audio_playSound(AUDIO_SFX_GAME_JOIN);
					firstFrame = 0;
					nextCaveNoise = currentTime + 120 + 120 * ((float)rand() / (float)RAND_MAX);
				}

				if (currentTime > nextCaveNoise)
				{
					AUDIO_PLAY_RANDOM_CAVE_NOISE();
					nextCaveNoise = currentTime + 120 + 120 * ((float)rand() / (float)RAND_MAX);
				}

				pthread_mutex_lock(&mutex_cum);
				previousCumPosition = cum.position;
				cum.position = vec3_sum(playerCollider->position, (vec3) { 0, 0.69f, 0 });

				cameraCollider->position= vec3_sum(playerCollider->position, (vec3) { 0, 0.69f, 0 });
				if (collider_getLastCollisionTag(cameraCollider) == CHUNK_COLLIDER_WATER)
					isCameraSubmerged = 69;
				else
					isCameraSubmerged = 0;

				//menu
				if (input_is_key_released(GLFW_KEY_ESCAPE))
					changeGameState(69, GAME_PAUSED);
				else if (input_is_key_released(GLFW_KEY_E))
					changeGameState(69, GAME_INVENTORY);
				else if (input_is_key_released(GLFW_KEY_T))
					changeGameState(69, GAME_COMMAND_LINE);


				//block interactions
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


						audio_playSound(AUDIO_SFX_BLOCK_BREAK);
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
									tempRaycastBlock[i] = tempChunkBounds[i] - 1;
									tempRaycastChunk[i]--;
								}
							}
						}
						
						if (0==collider_isInBounds(playerCollider, (vec3) { CHUNK_WIDTH* tempRaycastChunk[0] + tempRaycastBlock[0], CHUNK_HEIGHT * tempRaycastChunk[1] + tempRaycastBlock[1], CHUNK_WIDTH* tempRaycastChunk[2] + tempRaycastBlock[2] }, (vec3) { CHUNK_WIDTH* tempRaycastChunk[0] + tempRaycastBlock[0]+1, CHUNK_HEIGHT* tempRaycastChunk[1] + tempRaycastBlock[1]+1, CHUNK_WIDTH* tempRaycastChunk[2] + tempRaycastBlock[2]+1 }))
						{
							chunkManager_changeBlock(&cm, tempRaycastChunk[0], tempRaycastChunk[1], tempRaycastChunk[2], tempRaycastBlock[0], tempRaycastBlock[1], tempRaycastBlock[2], hotbarContent[hotbarSlotSelected]);
							chunkManager_reloadChunk(&cm, &mutex_cm, tempRaycastChunk[0], tempRaycastChunk[1], tempRaycastChunk[2]);
						
							audio_playSound(AUDIO_SFX_BLOCK_PLACE);
						}
					}
				}

				//look aroiund
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

				//hotbar input
				double scrolldx, scrolldy;
				input_get_mouse_scroll_delta(&scrolldx, &scrolldy);
				if (scrolldy < -0.001)
				{
					hotbarSlotSelected++;
					if (hotbarSlotSelected >= HOTBAR_SIZE)
						hotbarSlotSelected = 0;
				}
				if (scrolldy > 0.001)
				{
					hotbarSlotSelected--;
					if (hotbarSlotSelected < 0)
						hotbarSlotSelected = HOTBAR_SIZE-1;
				}
				for (int i = 0; i < HOTBAR_SIZE; i++)
					if (input_is_key_pressed(GLFW_KEY_1 + i))
						hotbarSlotSelected = i;

				//movement
				vec3 velocity = (vec3){ 0,0,0 };
				vec3 forward = vec3_normalize(vec3_create2(cum.front.x, 0, cum.front.z));

				switch (currentPlayerState)
				{
				case PLAYER_MORTAL:
					if (input_is_key_down(GLFW_KEY_W))
						velocity = vec3_sum(velocity, vec3_scale(forward, MOVEMENT_SPEED));
					if (input_is_key_down(GLFW_KEY_S))
						velocity = vec3_sum(velocity, vec3_scale(forward, -MOVEMENT_SPEED));
					if (input_is_key_down(GLFW_KEY_A))
						velocity = vec3_sum(velocity, vec3_scale(cum.right, -MOVEMENT_SPEED));
					if (input_is_key_down(GLFW_KEY_D))
						velocity = vec3_sum(velocity, vec3_scale(cum.right, MOVEMENT_SPEED));

					velocity.y = playerCollider->velocity.y;
					velocity.y += GRAVITY * PHYSICS_UPDATE;
					if (velocity.y < -70.0f)
						velocity.y = lerp(velocity.y, -70.0f, 0.1f);

					if (input_is_key_down(GLFW_KEY_SPACE))
					{
						if (COLLISION_GET_NEG_Y(playerCollider->flags))
						{
							velocity.y = JUMP_STRENGTH;
							audio_playSound(AUDIO_SFX_JUMP);
						}
					}
					break;

				case PLAYER_IMMORTAL:
					if (input_is_key_down(GLFW_KEY_W))
						velocity = vec3_sum(velocity, vec3_scale(forward, 10*MOVEMENT_SPEED));
					if (input_is_key_down(GLFW_KEY_S))
						velocity = vec3_sum(velocity, vec3_scale(forward, -10* MOVEMENT_SPEED));
					if (input_is_key_down(GLFW_KEY_A))
						velocity = vec3_sum(velocity, vec3_scale(cum.right, -10* MOVEMENT_SPEED));
					if (input_is_key_down(GLFW_KEY_D))
						velocity = vec3_sum(velocity, vec3_scale(cum.right, 10* MOVEMENT_SPEED));
					if (input_is_key_down(GLFW_KEY_LEFT_SHIFT))
						velocity = vec3_sum(velocity, (vec3) { 0, -10* MOVEMENT_SPEED, 0 });
					if (input_is_key_down(GLFW_KEY_SPACE))
						velocity = vec3_sum(velocity, (vec3) { 0, 10* MOVEMENT_SPEED, 0 });
					break;
				}

				playerCollider->velocity = velocity;

				//optifine zoom
				float fov = cum.fov;
				if (input_is_key_down(GLFW_KEY_C))
					fov = lerp(fov, 15.0f, 0.1f);
				else
					fov = lerp(fov, FOV, 0.1f);
				camera_setProjection(&cum, fov, window_getAspect(), CLIP_NEAR, CLIP_FAR);

				//player part done


				//physics update
				double time = glfwGetTime();
				physicsSystem_resetCollisions(ps);
				for (int i = 0; i < PHYSICS_STEPS_PER_UPDATE; i++)
				{
					physicsSystem_update(ps, PHYSICS_UPDATE / PHYSICS_STEPS_PER_UPDATE);
				}
				lastRaycastHit = rh.position;
				rh.position = (vec3){ 0,100000,0 };
				raycastFound = physicsSystem_raycast(ps, cum.position, cum.front, 6, 0.01f, &rh);

				rh.position = vec3_sum(rh.position, vec3_scale(rh.normal, -0.02f));
				rh.position = vec3_sum(rh.position, vec3_scale(cum.front, 0.02f));
				chunk_getChunkFromPos(rh.position, &raycastChunkX, &raycastChunkY, &raycastChunkZ);
				raycastBlockX = (int)(rh.position.x - CHUNK_WIDTH * raycastChunkX);
				raycastBlockY = (int)(rh.position.y - CHUNK_HEIGHT * raycastChunkY);
				raycastBlockZ = (int)(rh.position.z - CHUNK_WIDTH * raycastChunkZ);
				//printf("%.2f %.2f %.2f | %d %d %d\n", rh.position.x, rh.position.y, rh.position.z, CHUNK_WIDTH* raycastChunkX + raycastBlockX, CHUNK_HEIGHT* raycastChunkY + raycastBlockY, CHUNK_WIDTH* raycastChunkZ + raycastBlockZ);

				//update frustum culling
				pthread_mutex_lock(&mutex_cum);
				mat4 pv = mat4_multiply(cum.projection_matrix, cum.view_matrix);
				pthread_mutex_unlock(&mutex_cum);

				pthread_mutex_lock(&mutex_cm);
				chunkManager_calculateFrustumCull(&cm, &pv);
				pthread_mutex_unlock(&mutex_cm);

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
				break;

			case GAME_PAUSED:
				if (input_is_key_released(GLFW_KEY_ESCAPE))
					changeGameState(69, GAME_INGAME);
				break;

			case GAME_INVENTORY:
				if (input_is_key_released(GLFW_KEY_E))
					changeGameState(69, GAME_INGAME);
				else if (input_is_key_released(GLFW_KEY_ESCAPE))
					changeGameState(69, GAME_INGAME);

				for (int i = 0; i < HOTBAR_SIZE && inventoryDropTarget != -1; i++)
				{
					if (input_is_key_released(GLFW_KEY_1 + i) == 0)
						continue;

					if (i == inventoryDropTarget - 100)//if true, the content of the hotbar slot wants to be swapped with itself
						continue;

					inventorySlotContentSwap[0] = inventoryDropTarget;
					inventorySlotContentSwap[1] = 100 + i;

					break;//only one swap can be registered per frame anyway
				}
				break;

			case GAME_COMMAND_LINE:
				if (input_is_key_released(GLFW_KEY_ESCAPE))
				{
					pthread_mutex_lock(&mutex_vaszonCommand);
					strcpy(currentCommand, "");
					currentCommandWidth = canvas_calculateTextLength(vaszonCommand, currentCommand, COMMAND_LINE_FONT_SIZE);
					canvas_setComponentPosition(vaszonCommand, vaszonCommand_commandCursor, 15 + currentCommandWidth, 15);
					pthread_mutex_unlock(&mutex_vaszonCommand);

					changeGameState(69, GAME_INGAME);
					break;
				}
				if (input_is_key_released(GLFW_KEY_ENTER))
				{
					int commandType = COMMAND_NOT_COMMAND;
					int arguments[COMMAND_LINE_MAX_ARGUMENT_COUNT];
					int argumentCount = 0;
					
					convertCommandLine(currentCommand, &commandType, &argumentCount, arguments);

					char commandToShow[100];//the command that should be written into the history
					strcpy(commandToShow, "");

					switch (commandType)
					{
					case COMMAND_GAMEMODE:
						switch (arguments[0])
						{
						case PLAYER_MORTAL:
							collider_setSolidity(playerCollider, 69);
							currentPlayerState = PLAYER_MORTAL;
							printf("gamemode set to mortal\n");
							strcpy(commandToShow, "gamemode set to mortal");
							break;

						case PLAYER_IMMORTAL:
							collider_setSolidity(playerCollider, 0);
							currentPlayerState = PLAYER_IMMORTAL;
							printf("gamemode set to divine\n");
							strcpy(commandToShow, "gamemode set to divine");
							break;

						default:
							printf("invalid argument(s)\n");
							strcpy(commandToShow, "invalid argument(s)");
							break;
						}
						break;

					case COMMAND_SPEED:
						if (argumentCount == 1)
						{
							MOVEMENT_SPEED = arguments[0]*0.1f;
							printf("movement speed set to %.1f\n", MOVEMENT_SPEED);
							sprintf(commandToShow, "movement speed set to %.1f", MOVEMENT_SPEED);
						}
						else
						{
							printf("invalid argument(s)\n");
							strcpy(commandToShow, "invalid argument(s)");
						}
						break;

					case COMMAND_GRAVITY:
						if (argumentCount == 1)
						{
							GRAVITY = -0.1f*arguments[0];
							printf("gravity set to %.1f\n", -GRAVITY);
							sprintf(commandToShow, "gravity set to %.1f", -GRAVITY);
						}
						else
						{
							printf("invalid argument(s)\n");
							strcpy(commandToShow, "invalid argument(s)");
						}
						break;

					case COMMAND_JUMP:
						if (argumentCount == 1)
						{
							JUMP_STRENGTH = 0.1f * arguments[0];
							printf("jump strength set to %.1f\n", JUMP_STRENGTH);
							sprintf(commandToShow, "jump strength set to %.1f", JUMP_STRENGTH);
						}
						else
						{
							printf("invalid argument(s)\n");
							strcpy(commandToShow, "invalid argument(s)");
						}
						break;

					case COMMAND_TELEPORT:
						if (argumentCount == 2)
						{
							playerCollider->position = (vec3){ arguments[0], playerCollider->position.y, arguments[1] };
							printf("warped to %.0f, %.0f, %.0f\n", playerCollider->position.x, playerCollider->position.y, playerCollider->position.z);
							sprintf(commandToShow, "warped to %.0f, %.0f, %.0f", playerCollider->position.x, playerCollider->position.y, playerCollider->position.z);
						}
						else if (argumentCount == 3)
						{
							playerCollider->position = (vec3){ arguments[0], arguments[1], arguments[2]};
							printf("warped to %.0f, %.0f, %.0f\n", playerCollider->position.x, playerCollider->position.y, playerCollider->position.z);
							sprintf(commandToShow, "warped to %.0f, %.0f, %.0f", playerCollider->position.x, playerCollider->position.y, playerCollider->position.z);
						}
						else
						{
							printf("invalid argument(s)\n");
							strcpy(commandToShow, "invalid argument(s)");
						}
						break;

					case COMMAND_TIME:
						if (argumentCount == 0)
						{
							printf("too few arguments\n");
							strcpy(commandToShow, "too few arguments");
						}
						else if (argumentCount == 1)
						{
							arguments[0] %= 2400;
							TIME_OF_DAY = (float)arguments[0] / 2400.0f;
							printf("time set to %d\n", arguments[0]);
							sprintf(commandToShow, "time set to %d", arguments[0]);
						}
						else
						{
							printf("invalid argument(s)\n");
							strcpy(commandToShow, "invalid argument(s)");
						}
						break;

					case COMMAND_UNKNOWN:
						printf("invalid command\n");
						strcpy(commandToShow, "invalid command");
						break;

					case COMMAND_NOT_COMMAND:
						printf("%s\n", currentCommand);
						strcpy(commandToShow, currentCommand);
						break;

					default:
						printf("something's wrong with the command interpreter\n");
						strcpy(commandToShow, "an error occured while analysing input");
						break;
					}

					pthread_mutex_lock(&mutex_vaszonCommand);
					if (strcmp(commandToShow, "") != 0)
						updateCommandHistory(commandToShow);

					strcpy(currentCommand, "");
					currentCommandWidth = canvas_calculateTextLength(vaszonCommand, currentCommand, COMMAND_LINE_FONT_SIZE);
					canvas_setComponentPosition(vaszonCommand, vaszonCommand_commandCursor, 15 + currentCommandWidth, 15);
					pthread_mutex_unlock(&mutex_vaszonCommand);

					changeGameState(69, GAME_INGAME);
					break;
				}

				pthread_mutex_lock(&mutex_vaszonCommand);
				//the printable characters are detected and added with/in a callback function (command_line_character_callback)
				//the callback function is only active when the command line is open
				//backslash is not detected in the callback function
				if (input_is_key_pressed(GLFW_KEY_BACKSPACE) && strlen(currentCommand) > 0)
				{
					currentCommand[strlen(currentCommand) - 1] = '\0';
					currentCommandWidth = canvas_calculateTextLength(vaszonCommand, currentCommand, COMMAND_LINE_FONT_SIZE);
					canvas_setComponentPosition(vaszonCommand, vaszonCommand_commandCursor, 15 + currentCommandWidth, 15);
				}

				//flashing cursor
				if (((int)(1.5f*glfwGetTime())) % 2)
					canvas_setImageTint(vaszonCommand, vaszonCommand_commandCursor, CANVAS_COLOUR_ACCENT_0);
				else
					canvas_setImageTint(vaszonCommand, vaszonCommand_commandCursor, CANVAS_COLOUR_PRIMARY_1);
				pthread_mutex_unlock(&mutex_vaszonCommand);
				break;
		}

		//day-night cycle
		TIME_OF_DAY += PHYSICS_UPDATE / LENGTH_OF_DAY_IN_SECONDS;
		if (TIME_OF_DAY > 1.0f)
			TIME_OF_DAY -= 1.0f;
		sunDirection.x = 0;
		sunDirection.y = sinf(2*3.14159265f * TIME_OF_DAY);
		sunDirection.z = cosf(2 * 3.14159265f * TIME_OF_DAY);


		pthread_mutex_lock(&mutex_input);
		shouldPoll = 69;
		pthread_mutex_unlock(&mutex_input);

		physicsUpdateTime = glfwGetTime() - currentTime;

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

			if (shouldExit2 != EXIT_STATUS_REMAIN || !shouldPoll2)
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

		pthread_mutex_lock(&mutex_cum);
		camera_setProjection(&cum, currentFov, window_getAspect(), CLIP_NEAR, CLIP_FAR);
		pthread_mutex_unlock(&mutex_cum);
		//azert nem itt allitom at a text_renderer meretet, mert ez nem az opengl szal
		break;
	default:
		input_handle_event(e);
		break;
	}
}


void init_renderer()
{
	int fogEnd = settings_getInt(SETTINGS_RENDER_DISTANCE) * CHUNK_WIDTH - 10;
	int shadowEnd = fogEnd > 250 ? 250 : fogEnd;

	//rendor = renderer_create(window_getWidth(), window_getHeight());
	renderer_setWidth(settings_getInt(SETTINGS_RENDERER_WIDTH));
	renderer_setHeight(settings_getInt(SETTINGS_RENDERER_HEIGHT));
	renderer_setShadowResolution(settings_getInt(SETTINGS_SHADOW_RESOLUTION_PIXELS));
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
	//glUniform1f(glGetUniformLocation(waterShader.id, "fogExponent"), -0.00015f * 16.0f / settings_getInt(SETTINGS_RENDER_DISTANCE));
	//glUniform3f(glGetUniformLocation(waterShader.id, "fogColour"), 5, 5, 5);
	glUniform1f(glGetUniformLocation(waterShader.id, "fogStart"), fogEnd - 10);
	glUniform1f(glGetUniformLocation(waterShader.id, "fogEnd"), fogEnd);
	glUniform1f(glGetUniformLocation(waterShader.id, "fogHelper"), 0.1f);
	glUniform1f(glGetUniformLocation(waterShader.id, "projectionNear"), CLIP_NEAR);
	glUniform1f(glGetUniformLocation(waterShader.id, "projectionFar"), CLIP_FAR);
	glUniform1f(glGetUniformLocation(waterShader.id, "onePerScreenWidth"), 1.0f / RENDERER_WIDTH);
	glUniform1f(glGetUniformLocation(waterShader.id, "onePerScreenHeight"), 1.0f / RENDERER_HEIGHT);
	glUniform1f(glGetUniformLocation(waterShader.id, "shadowStart"), shadowEnd-10);
	glUniform1f(glGetUniformLocation(waterShader.id, "shadowEnd"), shadowEnd);
	glUniform1f(glGetUniformLocation(waterShader.id, "shadowHelper"), 1.0f / 10);


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
	//glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogExponent"), -0.00015f*16.0f/settings_getInt(SETTINGS_RENDER_DISTANCE));
	//glUniform3f(glGetUniformLocation(lightingPassShader.id, "fogColour"), 5, 5, 5);
	glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogStart"), fogEnd - 10);
	glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogEnd"), fogEnd);
	glUniform1f(glGetUniformLocation(lightingPassShader.id, "fogHelper"), 0.1f);
	glUniform1f(glGetUniformLocation(lightingPassShader.id, "shadowStart"), shadowEnd-10);
	glUniform1f(glGetUniformLocation(lightingPassShader.id, "shadowEnd"), shadowEnd);
	glUniform1f(glGetUniformLocation(lightingPassShader.id, "shadowHelper"), 1.0f / 10);

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
	glUniform1i(glGetUniformLocation(finalPassShader.id, "isSubmerged"), 0);

	finalPassNonHdrShader = shader_import(
		"../assets/shaders/renderer/final_pass_non_hdr/shader_final_non_hdr.vag",
		"../assets/shaders/renderer/final_pass_non_hdr/shader_final_non_hdr.fag",
		NULL
	);
	glUseProgram(finalPassNonHdrShader.id);
	glUniform1i(glGetUniformLocation(finalPassNonHdrShader.id, "tex"), 0);
	glUniform2f(glGetUniformLocation(finalPassNonHdrShader.id, "onePerResolution"), 1.0f / RENDERER_WIDTH, 1.0f / RENDERER_HEIGHT);

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

	moonTzu=light_create(
		vec3_create2(1, 1, 1),
		vec3_create2(-0.6, -0.5, 0.8),
		vec3_create2(1, 0, 0)
	);

	//stars

	nightSky = stars_create(300);

	//skybox
	skyboxShader = shader_import(
		"../assets/shaders/sky_procedural/shader_sky_procedural.vag",
		"../assets/shaders/sky_procedural/shader_sky_procedural.fag",
		NULL
	);
	glUseProgram(skyboxShader.id);
	glUniform1i(glGetUniformLocation(skyboxShader.id, "texture_gradient"), 0);
	glUniform1i(glGetUniformLocation(skyboxShader.id, "texture_gradient_colour_0"), 1);
	glUniform1i(glGetUniformLocation(skyboxShader.id, "texture_gradient_colour_1"), 2);
	glUniform1i(glGetUniformLocation(skyboxShader.id, "texture_gradient_horizon"), 3);
	glUniform3f(glGetUniformLocation(skyboxShader.id, "sunDirectionNormalized"), sunTzu.position.x, sunTzu.position.y, sunTzu.position.z);
	glUniform3f(glGetUniformLocation(skyboxShader.id, "sunColour"), 1, 1, 1);
	glUseProgram(0);

	skyboxMesh = kuba_create();


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
	shader_delete(&finalPassNonHdrShader);

	glDeleteVertexArrays(1, &rectangleVAO);
	glDeleteBuffers(1, &rectangleVBO);

	//light cleanup
	for (int i = 0; i < lights->size; i++)
		free((light*)vector_get(lights, i));
	vector_destroy(lights);

	light_destroyRenderer(lightRenderer);

	//stars
	stars_destroy(nightSky);

	//skybox
	shader_delete(&skyboxShader);
	mesh_destroy(skyboxMesh);

	//lens flare
	flare_destroy(lensFlare);

	//player mesh
	playerMesh_destroy(&pm);
}

void init_canvas()
{
	int mogus;

	//ingame screen
	vaszonIngame = canvas_create(window_getWidth(), window_getHeight(), "../assets/fonts/Monocraft.ttf");

	mogus = canvas_addImage(vaszonIngame, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, 0, 16, 16, textureHandler_getTexture(TEXTURE_ATLAS_UI));
	canvas_setImageUV(vaszonIngame, mogus, 0.0f, 0.9f, 0.1f, 0.1f);

	vaszonIngame_selectedBlockText = canvas_addText(vaszonIngame, "", CANVAS_ALIGN_CENTER, CANVAS_ALIGN_BOTTOM, 0, 90, CANVAS_COLOUR_ACCENT_0, 24);

	for (int i = 0; i < HOTBAR_SIZE; i++)
	{
		vaszonIngame_hotbarSlotFrames[i] = canvas_addButton(vaszonIngame, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_BOTTOM, -80 * (HOTBAR_SIZE / 2) + 80 * i,5, 70, 70);//player bg
		canvas_setButtonBorder(vaszonIngame, vaszonIngame_hotbarSlotFrames[i], 5, 5);
		canvas_setButtonFillColour(vaszonIngame, vaszonIngame_hotbarSlotFrames[i], 0, 0, 0);
		canvas_setButtonBorderColour(vaszonIngame, vaszonIngame_hotbarSlotFrames[i], CANVAS_COLOUR_PRIMARY_1);
		
		vaszonIngame_hotbarSlotBlockMeshes[i] = canvas_addBlockMesh(vaszonIngame, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_BOTTOM, hotbarContent[i], -80 * (HOTBAR_SIZE / 2) + 80 * i, 10, 60, 60);
	}

	//pause screen
	vaszonPause = canvas_create(window_getWidth(), window_getHeight(), "../assets/fonts/Monocraft.ttf");
	
	mogus = canvas_addButton(vaszonPause, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, 0, 300, 50);
	canvas_setButtonText(vaszonPause, mogus, "continue", 24, CANVAS_COLOUR_ACCENT_0);
	canvas_setButtonBorder(vaszonPause, mogus, 5, 5);
	canvas_setButtonFillColour(vaszonPause, mogus, CANVAS_COLOUR_PRIMARY_1);
	canvas_setButtonBorderColour(vaszonPause, mogus, CANVAS_COLOUR_PRIMARY_0);
	canvas_setButtonClicked(vaszonPause, mogus, changeGameState, (void*)GAME_INGAME);

	mogus = canvas_addButton(vaszonPause, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, -70, 300, 50);
	canvas_setButtonText(vaszonPause, mogus, "main menu", 24, CANVAS_COLOUR_ACCENT_0);
	canvas_setButtonBorder(vaszonPause, mogus, 5, 5);
	canvas_setButtonFillColour(vaszonPause, mogus, CANVAS_COLOUR_PRIMARY_1);
	canvas_setButtonBorderColour(vaszonPause, mogus, CANVAS_COLOUR_PRIMARY_0);
	canvas_setButtonClicked(vaszonPause, mogus, quitGame, NULL);

	canvas_addImage(vaszonPause, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_TOP, 0, 100, 630, 100, textureHandler_getTexture(TEXTURE_MENU_TITLE_PAUSE));

	//inventory
	vaszonInventory = canvas_create(window_getWidth(), window_getHeight(), "../assets/fonts/Monocraft.ttf");
	float inventoryTextLineHeight24 = canvas_getTextLineHeight(vaszonInventory, 24);

	mogus = canvas_addButton(vaszonInventory, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 0, 0, 600, 510);//bg
	canvas_setButtonBorder(vaszonInventory, mogus, 5, 5);
	canvas_setButtonFillColour(vaszonInventory, mogus, CANVAS_COLOUR_PRIMARY_1);
	canvas_setButtonBorderColour(vaszonInventory, mogus, CANVAS_COLOUR_PRIMARY_0);

	canvas_addText(vaszonInventory, "inventory", CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, -270 + 0.5f * canvas_calculateTextLength(vaszonInventory, "inventory", 24), 245 - 0.5f*inventoryTextLineHeight24, 1, 0.85f, 0, 24);//text in the top left


	mogus = canvas_addButton(vaszonInventory, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, -160, -0.5f*canvas_getTextLineHeight(vaszonInventory, 24), 220, 445);//player bg
	canvas_setButtonBorder(vaszonInventory, mogus, 5, 5);
	canvas_setButtonFillColour(vaszonInventory, mogus, 0, 0, 0);
	canvas_setButtonBorderColour(vaszonInventory, mogus, CANVAS_COLOUR_PRIMARY_0);

	canvas_addImage(vaszonInventory, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, -160, -0.5f * inventoryTextLineHeight24, 220, 445, textureHandler_getTexture(TEXTURE_GOKU));

	for (int i = 0; i < INVENTORY_ROWS; i++)
	{
		for (int j = 0; j < INVENTORY_COLUMNS; j++)
		{
			mogus = canvas_addButton(vaszonInventory, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, 20+75*j, 187.5-75*i-0.5f * inventoryTextLineHeight24, 70, 70);//player bg
			canvas_setButtonBorder(vaszonInventory, mogus, 5, 5);
			canvas_setButtonFillColour(vaszonInventory, mogus, 0, 0, 0);
			canvas_setButtonBorderColour(vaszonInventory, mogus, CANVAS_COLOUR_PRIMARY_0);

			canvas_setButtonPressed(vaszonInventory, mogus, inventorySlotPress, i * INVENTORY_COLUMNS + j);
			canvas_setButtonClicked(vaszonInventory, mogus, inventorySlotRelease, i * INVENTORY_COLUMNS + j);
			canvas_setButtonEnter(vaszonInventory, mogus, inventorySlotEnter, i * INVENTORY_COLUMNS + j);
			canvas_setButtonExit(vaszonInventory, mogus, inventorySlotExit, i * INVENTORY_COLUMNS + j);
			vaszonInventory_inventorySlotFrames[i * INVENTORY_COLUMNS + j] = mogus;

			vaszonInventory_inventorySlotBlockMeshes[i * INVENTORY_COLUMNS + j] = canvas_addBlockMesh(vaszonInventory, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, inventoryContent[i * INVENTORY_COLUMNS + j], 20 + 75 * j, 187.5 - 75 * i - 0.5f * inventoryTextLineHeight24, 60, 60);
		}
	}

	for (int i = 0; i < HOTBAR_SIZE; i++)
	{
		mogus = canvas_addButton(vaszonInventory, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_BOTTOM, -80 * (HOTBAR_SIZE / 2) + 80 * i, 5, 70, 70);//player bg
		canvas_setButtonBorder(vaszonInventory, mogus, 5, 5);
		canvas_setButtonFillColour(vaszonInventory, mogus, 0, 0, 0);
		canvas_setButtonBorderColour(vaszonInventory, mogus, CANVAS_COLOUR_PRIMARY_0);

		canvas_setButtonPressed(vaszonInventory, mogus, inventorySlotPress, 100+i);
		canvas_setButtonClicked(vaszonInventory, mogus, inventorySlotRelease, 100 + i);
		canvas_setButtonEnter(vaszonInventory, mogus, inventorySlotEnter, 100 + i);
		canvas_setButtonExit(vaszonInventory, mogus, inventorySlotExit, 100 + i);

		mogus = vaszonInventory_inventoryHotbarSlotFrames[i];


		vaszonInventory_inventoryHotbarSlotBlockMeshes[i] = canvas_addBlockMesh(vaszonInventory, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_BOTTOM, hotbarContent[i], -80 * (HOTBAR_SIZE / 2) + 80 * i, 10, 60, 60);
	}

	vaszonInventory_dragged = canvas_addBlockMesh(vaszonInventory, CANVAS_ALIGN_CENTER, CANVAS_ALIGN_MIDDLE, BLOCK_AIR, -100000, -100000, 60, 60);

	//command screen
	vaszonCommand = canvas_create(window_getWidth(), window_getHeight(), "../assets/fonts/Monocraft.ttf");

	mogus = canvas_addImage(vaszonCommand, CANVAS_ALIGN_LEFT, CANVAS_ALIGN_BOTTOM, 10, 10, 600, 30, textureHandler_getTexture(TEXTURE_ATLAS_UI));
	canvas_setImageTint(vaszonCommand, mogus, CANVAS_COLOUR_PRIMARY_1);
	canvas_setImageUV(vaszonCommand, mogus, 0, 0, 0.1f, 0.1f);

	vaszonCommand_commandCursor = canvas_addImage(vaszonCommand, CANVAS_ALIGN_LEFT, CANVAS_ALIGN_BOTTOM, 15, 15, 3, 20, textureHandler_getTexture(TEXTURE_ATLAS_UI));
	canvas_setImageTint(vaszonCommand, vaszonCommand_commandCursor, CANVAS_COLOUR_ACCENT_0);
	canvas_setImageUV(vaszonCommand, vaszonCommand_commandCursor, 0, 0, 0.1f, 0.1f);

	vaszonCommand_commandText = canvas_addText(vaszonCommand, "", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_BOTTOM, 15, 17, CANVAS_COLOUR_ACCENT_0, COMMAND_LINE_FONT_SIZE);

	for (int i = 0; i < COMMAND_LINE_HISTORY_LENGTH; i++)
	{
		vaszonCommand_commandHistory[i] = canvas_addText(vaszonCommand, "", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_BOTTOM, 15, 47 + i * 25, CANVAS_COLOUR_ACCENT_0, COMMAND_LINE_FONT_SIZE);
		strcpy(commandHistory[i], "");
	}
	indexOfMostRecentCommand = 0;

	//debug screen
	vaszon = canvas_create(window_getWidth(), window_getHeight(), "../assets/fonts/Monocraft.ttf");

	//left side
	vaszon_fps = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 15, CANVAS_COLOUR_PRIMARY_1, 24);
	vaszon_pos = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 40, CANVAS_COLOUR_PRIMARY_1, 24);
	vaszon_rot = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 65, CANVAS_COLOUR_PRIMARY_1, 24);

	vaszon_render_distance = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 105, CANVAS_COLOUR_PRIMARY_1, 24);
	vaszon_chunk_updates = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 130, CANVAS_COLOUR_PRIMARY_1, 24);
	vaszon_chunks_pending_updates = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 155, CANVAS_COLOUR_PRIMARY_1, 24);
	vaszon_chunks_loaded = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 180, CANVAS_COLOUR_PRIMARY_1, 24);
	vaszon_chunks_rendered = canvas_addText(vaszon, "alma", CANVAS_ALIGN_LEFT, CANVAS_ALIGN_TOP, 15, 205, CANVAS_COLOUR_PRIMARY_1, 24);

	//bototm left
	const char* vendor = glGetString(GL_VENDOR);
	const char* renderer = glGetString(GL_RENDERER);

	canvas_addText(vaszon, vendor, CANVAS_ALIGN_LEFT, CANVAS_ALIGN_BOTTOM, 15, 35, CANVAS_COLOUR_PRIMARY_1, 24);
	canvas_addText(vaszon, renderer, CANVAS_ALIGN_LEFT, CANVAS_ALIGN_BOTTOM, 15, 10, CANVAS_COLOUR_PRIMARY_1, 24);

	//right side
	vaszon_physics_update_time= canvas_addText(vaszon, "alma", CANVAS_ALIGN_RIGHT, CANVAS_ALIGN_TOP, 15, 10, CANVAS_COLOUR_PRIMARY_1, 24);
	vaszon_physics_loaded_groups = canvas_addText(vaszon, "alma", CANVAS_ALIGN_RIGHT, CANVAS_ALIGN_TOP, 15, 35, CANVAS_COLOUR_PRIMARY_1, 24);
	vaszon_physics_simulated_colliders = canvas_addText(vaszon, "alma", CANVAS_ALIGN_RIGHT, CANVAS_ALIGN_TOP, 15, 60, CANVAS_COLOUR_PRIMARY_1, 24);
	vaszon_physics_raycast_hit = canvas_addText(vaszon, "alma", CANVAS_ALIGN_RIGHT, CANVAS_ALIGN_TOP, 15, 85, CANVAS_COLOUR_PRIMARY_1, 24);
}

void end_canvas()
{
	canvas_destroy(vaszonIngame);
	canvas_destroy(vaszonPause);
	canvas_destroy(vaszonInventory);
	canvas_destroy(vaszonCommand);
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
		event_queue_push((event) { .type = KEY_PRESSED, .data.key_pressed = { key, mods } });
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

void command_line_character_callback(GLFWwindow* window, unsigned int codepoint)
{
	pthread_mutex_lock(&mutex_vaszonCommand);
	int length = strlen(currentCommand);
	if (isprint(codepoint) == 0 || length >= 99)
	{
		pthread_mutex_unlock(&mutex_vaszonCommand);
		return;
	}

	currentCommand[length] = codepoint;
	currentCommand[length + 1] = '\0';

	currentCommandWidth = canvas_calculateTextLength(vaszonCommand, currentCommand, COMMAND_LINE_FONT_SIZE);
	canvas_setComponentPosition(vaszonCommand, vaszonCommand_commandCursor, 15 + currentCommandWidth, 15);

	pthread_mutex_unlock(&mutex_vaszonCommand);
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

//command line things
void convertCommandLine(const char* command, int* commandType, int* argumentCount, int* arguments)
{
	*commandType = COMMAND_NOT_COMMAND;
	*argumentCount = 0;


	if (currentCommand==NULL||strlen(currentCommand) == 0 || command[0] != '/')
		return;

	int commandLength = strlen(currentCommand);
	char* wholeCommand = malloc((commandLength + 1) * sizeof(char));//copying the command so that it can be modified if needed
	strcpy(wholeCommand, currentCommand);



	//splitting up command
	static char words[COMMAND_LINE_MAX_ARGUMENT_COUNT+1][30];

	int inWord = 0;
	int wordStartIndex = 0;
	int wordCount = 0;
	for (int i = 0; i <= commandLength&&wordCount< COMMAND_LINE_MAX_ARGUMENT_COUNT + 1; i++)
	{
		if (wholeCommand[i] == ' ' || wholeCommand[i] == '\t' || wholeCommand[i] == '\n'||wholeCommand[i]=='\0')//characters that mark the end of a command
		{
			if (inWord != 0)
			{
				char tempC = wholeCommand[i];
				wholeCommand[i] = '\0';
				strcpy_s(words[wordCount],30, wholeCommand + wordStartIndex);
				wholeCommand[i] = tempC;
				wordCount++;
			}
			inWord = 0;
			continue;
		}
		
		if (inWord == 0)
		{
			inWord = 69;
			wordStartIndex = i;
		}
	}

	free(wholeCommand);

	//fill arguments
	*argumentCount = wordCount - 1;

	if (strcmp(words[0], "/gm") == 0)
		*commandType = COMMAND_GAMEMODE;
	else if (strcmp(words[0], "/speed") == 0)
		*commandType = COMMAND_SPEED;
	else if (strcmp(words[0], "/jump_strength") == 0)
		*commandType = COMMAND_JUMP;
	else if (strcmp(words[0], "/gravity") == 0)
		*commandType = COMMAND_GRAVITY;
	else if (strcmp(words[0], "/warp") == 0)
		*commandType = COMMAND_TELEPORT;
	else if (strcmp(words[0], "/time") == 0)
		*commandType = COMMAND_TIME;
	else
		*commandType = COMMAND_UNKNOWN;

	//attempting to scan all of the command line arguments
	for (int i = 0; i < COMMAND_LINE_MAX_ARGUMENT_COUNT && i < wordCount - 1; i++)
	{
		arguments[i] = 0;
		sscanf(words[i + 1], "%d", arguments + i);
	}

	//check if the player wants to reset the value
	if (strcmp(words[1], "reset") == 0)
	{
		switch (*currentCommand)
		{
		case COMMAND_GAMEMODE:
			arguments[0] = lroundf(10*PLAYER_MORTAL);
			break;

		case COMMAND_SPEED:
			arguments[0] = lroundf(10 * MOVEMENT_SPEED_BASED);
			break;

		case COMMAND_GRAVITY:
			arguments[0] = lroundf(10 *GRAVITY_BASED);
			break;

		case COMMAND_JUMP:
			arguments[0] = lroundf(10 * JUMP_STRENGTH_BASED);
			break;

		case COMMAND_TIME:
			arguments[0] = 0;
			break;

		default:
			*argumentCount = 0;
			break;
		}
	}

	//recheck if the input type is special
	switch (*commandType)
	{
	case COMMAND_GAMEMODE:
		if (wordCount > 1)
		{
			if (strcmp(words[1], "mortal") == 0)
				arguments[0] = PLAYER_MORTAL;
			if (strcmp(words[1], "divine") == 0)
				arguments[0] = PLAYER_IMMORTAL;
		}
		break;

	case COMMAND_TIME:
		if (wordCount > 1)
		{
			if (strcmp(words[1], "sunrise") == 0)
				arguments[0] = 0;
			if (strcmp(words[1], "noon") == 0)
				arguments[0] = 600;
			if (strcmp(words[1], "sunset") == 0)
				arguments[0] = 1200;
			if (strcmp(words[1], "midnight") == 0)
				arguments[0] = 1800;
		}
		break;
	}
}

void updateCommandHistory(const char* newCommand)
{
	indexOfMostRecentCommand--;
	if (indexOfMostRecentCommand < 0)
		indexOfMostRecentCommand = COMMAND_LINE_HISTORY_LENGTH - 1;
	strcpy_s(commandHistory[indexOfMostRecentCommand], 100, newCommand);

	int index = indexOfMostRecentCommand;
	for (int i = 0; i < COMMAND_LINE_HISTORY_LENGTH; i++,index++)
	{
		if (index >= COMMAND_LINE_HISTORY_LENGTH)
			index = 0;

		canvas_setTextText(vaszonCommand, vaszonCommand_commandHistory[i], commandHistory[index]);
	}
}

//button callbacks
void changeGameState(int inBounds, int gs)
{
	if (inBounds == 0)
		return; 

	pthread_mutex_lock(&mutex_gameState);
	if (gs == currentGameState)
	{
		pthread_mutex_unlock(&mutex_gameState);
		return;
	}

	switch (currentGameState)
	{
	case GAME_INGAME:
		switch (gs)
		{
		case GAME_PAUSED:
		case GAME_INVENTORY:
			pthread_mutex_lock(&mutex_input);
			targetCursorMode = GLFW_CURSOR_NORMAL;
			pthread_mutex_unlock(&mutex_input);
			break;

		case GAME_COMMAND_LINE:
			glfwSetCharCallback(window, command_line_character_callback);//ez frissiti a command lineon az osszes irhato karaktert
			pthread_mutex_lock(&mutex_input);
			targetCursorMode = GLFW_CURSOR_NORMAL;
			pthread_mutex_unlock(&mutex_input);
			break;
		}
		break;
	case GAME_PAUSED:
		switch (gs)
		{
		case GAME_INGAME:
			pthread_mutex_lock(&mutex_input);
			targetCursorMode = GLFW_CURSOR_DISABLED;
			pthread_mutex_unlock(&mutex_input);
			break;
		}
		break;

	case GAME_INVENTORY:
		switch (gs)
		{
		case GAME_INGAME:
			pthread_mutex_lock(&mutex_input);
			targetCursorMode = GLFW_CURSOR_DISABLED;
			pthread_mutex_unlock(&mutex_input);
			break;
		}
		break;

	case GAME_COMMAND_LINE:
		glfwSetCharCallback(window, NULL);
		switch (gs)
		{
		case GAME_INGAME:
			pthread_mutex_lock(&mutex_input);
			targetCursorMode = GLFW_CURSOR_DISABLED;
			pthread_mutex_unlock(&mutex_input);
			break;
		}
		break;
	}
	currentGameState = gs;
	pthread_mutex_unlock(&mutex_gameState);
}

void quitGame(int inBounds, void* nichts)
{
	if (inBounds == 0)
		return;

	pthread_mutex_lock(&mutex_exit);
	exitStatus = EXIT_STATUS_RETURN_TO_MENU;
	pthread_mutex_unlock(&mutex_exit);
}


void inventorySlotPress(int inventorySlotID)
{
	draggedInventorySlot = inventorySlotID;
}

void inventorySlotRelease(int inBounds, int inventorySlotID)
{
	if (inBounds == 1||inventoryDropTarget==-1)//that means that there is no different slot to drop the thing
	{
		draggedInventorySlot = -1;
		inventoryDropTarget = -1;
		return;
	}

	pthread_mutex_lock(&mutex_vaszonInventory);
	inventorySlotContentSwap[0] = inventorySlotID;
	inventorySlotContentSwap[1] = inventoryDropTarget;
	draggedInventorySlot = -1;
	inventoryDropTarget = -1;
	pthread_mutex_unlock(&mutex_vaszonInventory);
}

void inventorySlotExit(int inventorySlotID)
{
	if (inventorySlotID == inventoryDropTarget)
		inventoryDropTarget = -1;
}

void inventorySlotEnter(int inventorySlotID)
{
	inventoryDropTarget = inventorySlotID;
}