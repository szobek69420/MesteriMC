#include "sun.h"

#include "../../camera/camera.h"
#include "../../mesh/mesh.h"
#include "../../glm2/vec3.h"
#include "../../glm2/mat3.h"
#include "../../glm2/mat4.h"
#include "../../shader/shader.h"
#include "../../texture_handler/texture_handler.h"

#include <stdlib.h>
#include <math.h>
#include <glad/glad.h>

#define DEG2RAD 0.01745329251f
#define RAD2DEG 57.295779513f

static float vertices[20];
static unsigned int indices[6];

sun sun_create()
{
	sun sunTzu;

	glGenVertexArrays(1, &sunTzu.meh.vao);
	glBindVertexArray(sunTzu.meh.vao);

	glGenBuffers(1, &sunTzu.meh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, sunTzu.meh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &sunTzu.meh.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sunTzu.meh.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);//position
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);//uv

	glBindVertexArray(0);

	sunTzu.meh.indexCount = 6;

	sunTzu.program = shader_import(
		"../assets/shaders/sun/shader_sun.vag",
		"../assets/shaders/sun/shader_sun.fag",
		NULL
	);
	glUseProgram(sunTzu.program.id);
	glUniform1i(glGetUniformLocation(sunTzu.program.id, "texture_sun"), 0);
	glUseProgram(0);

	sunTzu.model = mat4_create(1);
	sun_setDirection(&sunTzu, vec3_create2(0.6, 1.3, 0.8));

	return sunTzu;
}

void sun_destroy(sun* sunTzu)
{
	mesh_destroy(sunTzu->meh);
	shader_delete(&sunTzu->program);
}

void sun_render(sun* sunTzu, camera* cum, mat4* projection)
{
	mat4 sunView = camera_getViewMatrix(cum);
	//sunView.data[12] = 0;
	//sunView.data[13] = 0;
	//sunView.data[14] = 0;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_SUN));

	glUseProgram(sunTzu->program.id);
	glUniformMatrix4fv(glGetUniformLocation(sunTzu->program.id, "model"), 1, GL_FALSE, mat4_multiply(mat4_create2((float[]) {1,0,0,0,0,1,0,0,0,0,1,0,cum->position.x, cum->position.y, cum->position.z, 1}), sunTzu->model).data);
	glUniformMatrix4fv(glGetUniformLocation(sunTzu->program.id, "view"), 1, GL_FALSE, sunView.data);
	glUniformMatrix4fv(glGetUniformLocation(sunTzu->program.id, "projection"), 1, GL_FALSE, projection->data);

	glBindVertexArray(sunTzu->meh.vao);
	glDrawElements(GL_TRIANGLES, sunTzu->meh.indexCount, GL_UNSIGNED_INT, (void*)0);

	glUseProgram(0);
	glBindVertexArray(0);
}

void sun_setDirection(sun* sunTzu, vec3 direction)
{
	sunTzu->direction = vec3_normalize(direction);
	vec3 horizontal = vec3_normalize(vec3_create2(sunTzu->direction.x, 0, sunTzu->direction.z));
	
	float pitch, yaw;
	float dot = vec3_dot(sunTzu->direction, vec3_create2(0, 1, 0));
	if (dot > 0.999f)
	{
		pitch = 90;
		yaw = 0;
	}
	else if (dot < -0.999f)
	{
		pitch = 270;
		yaw = 0;
	}
	else
	{
		pitch = RAD2DEG * asinf(vec3_dot(sunTzu->direction, vec3_create2(0,1,0)));
		if (vec3_dot(direction, horizontal) < 0)
			pitch = 360 - pitch;
		yaw = RAD2DEG * acosf(vec3_dot(horizontal, vec3_create2(0, 0, -1)));
		if (vec3_dot(horizontal, vec3_create2(-1, 0, 0)) < 0)
			yaw = 360 - yaw;

		pitch += 180;
	}

	sunTzu->model = mat4_rotate(mat4_create(1), vec3_create2(-1, 0, 0), pitch);
	sunTzu->model = mat4_rotate(sunTzu->model, vec3_create2(0, 1, 0), yaw);
}

static float vertices[] = {
	-1*SUN_SIZE, -1 * SUN_SIZE, 100,	1, 0,
	SUN_SIZE, -1 * SUN_SIZE, 100,		0, 0,
	SUN_SIZE, SUN_SIZE, 100,			0, 1,
	-1 * SUN_SIZE, SUN_SIZE, 100,		1, 1
};

static unsigned int indices[] = {
	0, 1, 2,
	2, 3, 0
};