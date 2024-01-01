#include <glad/glad.h>
#include <stb_image.h>
#include <stdio.h>
#include "texture_handler.h"

static unsigned int atlas_albedo=0;
static unsigned int atlas_specular=0;//https://github.com/rre36/lab-pbr/wiki/Specular-Texture-Details (mi csak az r komponenst használjuk)
static unsigned int atlas_normal=0;//rgba(normal.x, normal.y, normal.z, height)

unsigned int textureHandler_loadImage(const char* pathToTexture, GLint internalFormat, GLenum format, int filterType, int flipVertically);

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

    return problem;
}

void textureHandler_destroyTextures()
{
	glDeleteTextures(1, &atlas_albedo);
	glDeleteTextures(1, &atlas_specular);
	glDeleteTextures(1, &atlas_normal);
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