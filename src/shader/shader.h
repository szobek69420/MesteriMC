#ifndef SHADER_H
#define SHADER_H

#define SHADER_MAX_LENGTH 10000

#include <glad/glad.h>

#include "../glm2/vec3.h"
#include "../glm2/mat4.h"

typedef struct {
	GLuint id;
	int isValid;
} shader;

shader shader_import(const char* vertex, const char* fragment, const char* geometry);
void shader_delete(shader* shader);

void shader_use(GLuint shaderID);
void shader_setInt(GLuint shaderID, const char* uniformName, int uniform);
void shader_setFloat(GLuint shaderID, const char* uniformName, float uniform);
void shader_setVec3(GLuint shaderID, const char* uniformName, float x, float y, float z);
void shader_setVec3v(GLuint shaderID, const char* uniformName, vec3 vec);
void shader_setMat4(GLuint shaderID, const char* uniformName, mat4 mat);

#endif
