#ifndef SHADER_H
#define SHADER_H

#define SHADER_MAX_LENGTH 10000

#include <glad/glad.h>

typedef struct {
	GLuint id;
	int isValid;
} shader;

shader importShader(const char *, const char *, const char *);
void deleteShader(shader*);

void setShaderInt(GLuint shaderID, const char* uniformName, int uniform);

#endif
