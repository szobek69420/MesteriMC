#include <glad/glad.h>
#include <stb_image.h>
#include <stdio.h>
#include "texture_handler.h"

static unsigned int atlas_albedo=0;
static unsigned int atlas_specular=0;//https://github.com/rre36/lab-pbr/wiki/Specular-Texture-Details (mi csak az r komponenst haszn�ljuk)
static unsigned int atlas_normal=0;//rgba(normal.x, normal.y, normal.z, height)

static unsigned int skybox = 0;
static unsigned int sun = 0;

unsigned int textureHandler_loadImage(const char* pathToTexture, GLint internalFormat, GLenum format, int filterType, int flipVertically);
unsigned int textureHandler_loadSkybox();

int textureHandler_importTextures()
{
    int problem = 0;
    atlas_albedo = textureHandler_loadImage("../assets/textures/atlas_albedo.png",GL_SRGB, GL_RGBA,GL_NEAREST,69);
    if (atlas_albedo == 0)
        problem++;

    atlas_specular = textureHandler_loadImage("../assets/textures/atlas_specular.png", GL_RGB, GL_RGBA, GL_NEAREST, 69);
    if (atlas_specular == 0)
        problem++;

    atlas_normal = textureHandler_loadImage("../assets/textures/atlas_normal.png", GL_RGBA, GL_RGBA, GL_NEAREST, 69);
    if (atlas_normal == 0)
        problem++;


    skybox = textureHandler_loadSkybox();
    if (skybox == 0)
        problem++;

    sun = textureHandler_loadImage("../assets/textures/sun/sun.png", GL_RGBA, GL_RGBA, GL_LINEAR, 69);
    if (sun == 0)
        problem++;

    return problem;
}

void textureHandler_destroyTextures()
{
	glDeleteTextures(1, &atlas_albedo);
	glDeleteTextures(1, &atlas_specular);
	glDeleteTextures(1, &atlas_normal);

    glDeleteTextures(1, &skybox);
    glDeleteTextures(1, &sun);
}

unsigned int textureHandler_getTexture(int texture)
{
    switch (texture)
    {
    case TEXTURE_ATLAS_ALBEDO:
        return atlas_albedo;

    case TEXTURE_ATLAS_SPECULAR:
        return atlas_specular;

    case TEXTURE_ATLAS_NORMAL:
        return atlas_normal;

    case TEXTURE_SKYBOX:
        return skybox;

    case TEXTURE_SUN:
        return sun;

    default:
        return 0;
    }
}


unsigned int textureHandler_loadImage(const char* pathToTexture, GLint internalFormat, GLenum format, int filterType, int flipVertically)
{
    stbi_set_flip_vertically_on_load(flipVertically);
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // load and generate the texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load(pathToTexture, &width, &height, &nrChannels, 0);
    if (data!=NULL)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterType);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterType);
    }
    else
    {
        printf("Failed to load the texture %s\n", pathToTexture);
        return 0;
    }
    stbi_image_free(data);

    return texture;
}

unsigned int textureHandler_loadSkybox()
{
    unsigned int cubeMap;
    unsigned char* data;
    int width, height, nrChannels;

    glGenTextures(1, &cubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

    stbi_set_flip_vertically_on_load(0);
    const char* cubeMapTextures[]={ 
        "../assets/textures/skybox/left.png",
        "../assets/textures/skybox/right.png",
        "../assets/textures/skybox/top.png",
        "../assets/textures/skybox/bottom.png",
        "../assets/textures/skybox/front.png",
        "../assets/textures/skybox/back.png" };

    for (unsigned int i = 0; i < 6; i++)
    {
        data = stbi_load(cubeMapTextures[i], &width, &height, &nrChannels, 0);
        
        if (data != NULL)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_SRGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            printf("The skybox texture \"%s\" is absolutely fucked\n", cubeMapTextures[i]);
            return 0;
        }
    }
    stbi_set_flip_vertically_on_load(1);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return cubeMap;
}