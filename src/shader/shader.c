#define _CRT_SECURE_NO_WARNINGS

#include "shader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glad/glad.h>

shader shader_import(const char* pathToVertexShader, const char* pathToFragmentShader, const char* pathToGeometryShader)
{
    shader program;
    program.id = 0;
    program.isValid = 0;

    int  success;
    char infoLog[512];

    FILE* file;
    char* sauce = (char*)malloc(SHADER_MAX_LENGTH*sizeof(char));
    int currentLength;
    char line[300];

    GLuint vertexShader, fragmentShader, geometryShader;

    //vertex sharter
    currentLength = 0;
    file = fopen(pathToVertexShader, "r");
    if (file == NULL)
    {
        printf("ERROR::SHADER::VERTEX::FILE_NOT_FOUND\n");
        free(sauce);
        return program;
    }
    while (fgets(line, 300, file) != NULL)
    {
        strcpy(sauce + currentLength, line);
        currentLength += strlen(line);
    }
    fclose(file);


    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const char*)&sauce, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
        printf("%s\n", infoLog);
        free(sauce);
        return program;
    }

    //geometry shader
    if (pathToGeometryShader != NULL) {
        currentLength = 0;
        file = fopen(pathToGeometryShader, "r");
        if (file == NULL)
        {
            printf("ERROR::SHADER::GEOMETRY::FILE_NOT_FOUND\n");
            free(sauce);
            return program;
        }
        while (fgets(line, 300, file) != NULL)
        {
            strcpy(sauce + currentLength, line);
            currentLength += strlen(line);
        }
        fclose(file);


        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, (const char*)&sauce, NULL);
        glCompileShader(geometryShader);

        glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
            printf("ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n");
            printf("%s\n", infoLog);
            free(sauce);
            return program;
        }
    }


    //fragment sharter
    currentLength = 0;
    file = fopen(pathToFragmentShader, "r");
    if (file == NULL)
    {
        printf("ERROR::SHADER::FRAGMENT::FILE_NOT_FOUND\n");
        free(sauce);
        return program;
    }
    while (fgets(line, 300, file) != NULL)
    {
        strcpy(sauce + currentLength, line);
        currentLength += strlen(line);
    }
    fclose(file);


    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const char*)&sauce, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
        printf("%s\n", infoLog);
        free(sauce);
        return program;
    }

    //linkin'
    program.id = glCreateProgram();
    glAttachShader(program.id, vertexShader);
    if (pathToGeometryShader != NULL)
        glAttachShader(program.id, geometryShader);
    glAttachShader(program.id, fragmentShader);
    glLinkProgram(program.id);

    glGetProgramiv(program.id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program.id, 512, NULL, infoLog);
        printf("ERROR::SHADER::LINKING::LINKING_FAILED");
        free(sauce);
        return program;
    }

    //garbazs kollekcio
    free(sauce);
    glDeleteShader(vertexShader);
    if (pathToGeometryShader != NULL)
        glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    program.isValid = 69;
    return program;
}
void shader_delete(shader* shaderToDelete)
{
    glDeleteShader(shaderToDelete->id);
}

void shader_use(GLuint shaderID)
{
    glUseProgram(shaderID);
}
void shader_setInt(GLuint shaderID, const char* uniformName, int uniform)
{
    glUseProgram(shaderID);
    glUniform1i(glGetUniformLocation(shaderID, uniformName), uniform);
}
void shader_setFloat(GLuint shaderID, const char* uniformName, float uniform)
{
    glUseProgram(shaderID);
    glUniform1f(glGetUniformLocation(shaderID, uniformName), uniform);

}
void shader_setVec3(GLuint shaderID, const char* uniformName, float x, float y, float z)
{
    glUseProgram(shaderID);
    glUniform3f(glGetUniformLocation(shaderID, uniformName), x, y, z);
}
void shader_setVec3v(GLuint shaderID, const char* uniformName, vec3 vec)
{
    glUseProgram(shaderID);
    glUniform3f(glGetUniformLocation(shaderID, uniformName), vec.x, vec.y, vec.z);
}
void shader_setMat4(GLuint shaderID, const char* uniformName, mat4 mat)
{
    glUseProgram(shaderID);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, uniformName), 1, GL_FALSE, mat.data);
}