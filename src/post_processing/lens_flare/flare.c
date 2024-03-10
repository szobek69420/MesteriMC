#include "flare.h"
#include "../../texture_handler/texture_handler.h"
#include "../../shader/shader.h"
#include "../../glm2/vec3.h"
#include "../../glm2/vec4.h"
#include "../../glm2/mat4.h"
#include "../../settings/settings.h"

#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>

struct flare {
	unsigned int vao, vbo;

	shader shaderFlare;
	unsigned int textures[FLARE_TEXTURE_COUNT];
	float spacing;

	unsigned int queries[FLARE_QUERY_COUNT];
	unsigned int waitedQuery;//index of the query we are currently waiting for
	unsigned int nextQuery;//index of the query we will use the next frame
	unsigned int queryResult;//the result of the last successful query
	shader shaderQuery;

	int FLARE_MAX_SAMPLES_PASSED;
};

const char* textures[] = {
	"../assets/textures/flare/tex1.png",
	"../assets/textures/flare/tex2.png",
	"../assets/textures/flare/tex3.png",
	"../assets/textures/flare/tex4.png",
	"../assets/textures/flare/tex5.png",
	"../assets/textures/flare/tex6.png",
	"../assets/textures/flare/tex7.png",
	"../assets/textures/flare/tex8.png",
	"../assets/textures/flare/tex9.png"
};

const int texture_render_count = 11;

const int texture_index_in_render_order[] = {
	5,3,1,6,2,4,6,2,4,3,7
};

const float texture_size_in_render_order[] = {
	1.0f, 0.46f, 0.2f, 0.1f, 0.12f, 0.14f, 0.4f, 0.14f, 0.6f, 0.8f, 1.2f
};

const float vertices[] = {
	0.5f, 0.5f,			1,1,
	0.5f, -0.5f,		1,0,
	-0.5f, -0.5f,		0,0,

	-0.5f, -0.5f,		0,0,
	-0.5f, 0.5f,		0,1,
	0.5f, 0.5f,			1,1
};

flare* flare_create(float spacing)
{
	flare* sus=malloc(sizeof(flare));
	
	//vertex data
	glGenVertexArrays(1, &sus->vao);
	glBindVertexArray(sus->vao);

	glGenBuffers(1, &sus->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, sus->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//flare part
	sus->spacing = spacing;
	sus->shaderFlare = shader_import("../assets/shaders/renderer/flare/shader_flare.vag", "../assets/shaders/renderer/flare/shader_flare.fag", NULL);
	glUseProgram(sus->shaderFlare.id);
	glUniform1i(glGetUniformLocation(sus->shaderFlare.id, "tex"), 0);
	glUseProgram(0);
	for (int i = 0; i < FLARE_TEXTURE_COUNT; i++)
		sus->textures[i] = textureHandler_loadImage(textures[i], GL_RGBA, GL_RGBA, GL_LINEAR, 69);

	//query part
	glGenQueries(FLARE_QUERY_COUNT, &sus->queries);
	for (int i = 0; i < FLARE_QUERY_COUNT; i++)
	{
		glBeginQuery(GL_SAMPLES_PASSED, sus->queries[i]);
		glEndQuery(GL_SAMPLES_PASSED);
	}
	sus->waitedQuery = 0;
	sus->nextQuery = 0;
	sus->queryResult = 0;
	sus->shaderQuery = shader_import("../assets/shaders/renderer/flare/shader_flare.vag", "../assets/shaders/renderer/flare/shader_flare_query.fag", NULL);

	flare_update(sus);

	return sus;
}

void flare_destroy(flare* sus)
{
	glDeleteBuffers(1, &sus->vbo);
	glDeleteVertexArrays(1, &sus->vao);

	shader_delete(&sus->shaderFlare);
	glDeleteTextures(FLARE_TEXTURE_COUNT, &sus->textures);

	shader_delete(&sus->shaderQuery);

	free(sus);
}

void flare_render(flare* sus, mat4* projectionView, vec3 cumPos, vec3 sunDir, float aspectYX)
{
	//render flare
	float brightness = 1;
	vec4 sunScreen = vec4_multiplyWithMatrix(*projectionView, vec4_create2(sunDir.x+cumPos.x, sunDir.y+cumPos.y, sunDir.z+cumPos.z, 1));
	if (sunScreen.w < 0)
		goto yeet;

	sunScreen = vec4_scale(sunScreen, 1 / sunScreen.w);//perspective division
	vec3 sunToCenter = vec3_create2(-sunScreen.x, -sunScreen.y, 0);
	brightness = 0.5f - (vec3_magnitude(sunToCenter) * 0.844444f);
	brightness *= (float)sus->queryResult / sus->FLARE_MAX_SAMPLES_PASSED;
	//printf("passed: %d of %d\n", sus->queryResult, sus->FLARE_MAX_SAMPLES_PASSED);

	if (brightness < 0)
		goto yeet;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glBindVertexArray(sus->vao);
	glUseProgram(sus->shaderFlare.id);
	glUniform1f(glGetUniformLocation(sus->shaderFlare.id, "brightness"), brightness);
	glActiveTexture(GL_TEXTURE0);
	vec3 spacing = vec3_scale(sunToCenter, sus->spacing);
	unsigned int pos, scale;
	pos = glGetUniformLocation(sus->shaderFlare.id, "pos");
	scale = glGetUniformLocation(sus->shaderFlare.id, "scale");
	float x, y;
	x = sunScreen.x;
	y = sunScreen.y;
	for (int i = 0; i < texture_render_count; i++)
	{
		x += spacing.x;
		y += spacing.y;
		glUniform2f(pos, x, y);
		glUniform2f(scale, aspectYX*texture_size_in_render_order[i], texture_size_in_render_order[i]);
		glBindTexture(GL_TEXTURE_2D, sus->textures[texture_index_in_render_order[i]]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glDisable(GL_BLEND);
	//glEnable(GL_DEPTH_TEST); //ha a screen fbo-t hasznalom, akkor ez semmikeppen sem maradjon bent, mert annak nincs depth buffer-je es furcsan beakad a program

yeet:
	brightness = 0;
	return;
}

void flare_queryQueryResult(flare* sus)
{
	sus->queryResult = 0;

	unsigned int isAvailable = 0;
	glGetQueryObjectuiv(sus->queries[sus->waitedQuery], GL_QUERY_RESULT_AVAILABLE, &isAvailable);
	if (isAvailable)
	{
		glGetQueryObjectiv(sus->queries[sus->waitedQuery], GL_QUERY_RESULT, (unsigned int*)&sus->queryResult);

		sus->waitedQuery++;
		if (sus->waitedQuery >= FLARE_QUERY_COUNT)
			sus->waitedQuery = 0;
	}
}

void flare_query(flare* sus, mat4* projectionView, vec3 cumPos, vec3 sunDir, float aspectYX)
{
	vec4 sunScreen = vec4_multiplyWithMatrix(*projectionView, vec4_create2(sunDir.x + cumPos.x, sunDir.y + cumPos.y, sunDir.z + cumPos.z, 1));
	sunScreen = vec4_scale(sunScreen, 1 / sunScreen.w);//perspective division

	glUseProgram(sus->shaderQuery.id);
	glUniform2f(glGetUniformLocation(sus->shaderQuery.id, "pos"), sunScreen.x, sunScreen.y);
	glUniform2f(glGetUniformLocation(sus->shaderQuery.id, "scale"), aspectYX*0.15f, 0.15f);

	glBeginQuery(GL_SAMPLES_PASSED, sus->queries[sus->nextQuery]);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBindVertexArray(sus->vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glEndQuery(GL_SAMPLES_PASSED);

	sus->nextQuery++;
	if (sus->nextQuery >= FLARE_QUERY_COUNT)
		sus->nextQuery = 0;

	glDepthFunc(GL_LESS);
	glBindVertexArray(0);
	glUseProgram(0);
}

void flare_update(flare* sus)
{
	sus->FLARE_MAX_SAMPLES_PASSED = FLARE_MAX_SAMPLES_PASSED_FULL_HD*(settings_getInt(SETTINGS_RENDERER_WIDTH)/1920.0f * settings_getInt(SETTINGS_RENDERER_HEIGHT)/1080.0f);
}