#include "sun.h"

#include "../../camera/camera.h"
#include "../../mesh/mesh.h"
#include "../../mesh/sprite/sprite_mesh.h"
#include "../../glm2/vec3.h"
#include "../../glm2/mat4.h"
#include "../../shader/shader.h"
#include "../../texture_handler/texture_handler.h"

#include <stdlib.h>
#include <glad/glad.h>

sun sun_create()
{
	sun sunTzu;
	sunTzu.meh = spriteMesh_create();
	sunTzu.direction = vec3_create2(0, 0, 1);
	sunTzu.program = shader_import(
		"../assets/shaders/sun/shader_sun.vag",
		"../assets/shaders/sun/shader_sun.fag",
		NULL
	);
	glUseProgram(sunTzu.program.id);
	glUniform1i(glGetUniformLocation(sunTzu.program.id, "texture_sun"), 0);
	glUseProgram(0);

	sunTzu.model = mat4_create2((float[]) { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -200, 1 });
	sunTzu.model = mat4_translate(mat4_create(1), vec3_create2(0, 0, 100));
	sunTzu.model = mat4_scale(sunTzu.model, vec3_create(SUN_SIZE));

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

void sun_setDirection(vec3 direction);