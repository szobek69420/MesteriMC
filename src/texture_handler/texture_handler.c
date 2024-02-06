#include <glad/glad.h>
#include <stb_image.h>
#include <stdio.h>
#include "texture_handler.h"

//ingame
static unsigned int atlas_albedo=0;
static unsigned int atlas_specular=0;//https://github.com/rre36/lab-pbr/wiki/Specular-Texture-Details (mi csak az r komponenst haszn�ljuk)
static unsigned int atlas_normal=0;//rgba(normal.x, normal.y, normal.z, height)

static unsigned int water_normal=0;
static unsigned int water_dudv=0;

static unsigned int skybox = 0;
static unsigned int sun = 0;
static unsigned int sky_gradient = 0;

//menu
static unsigned int menu_title = 0;
static unsigned int menu_background = 0;



unsigned int textureHandler_loadImage(const char* pathToTexture, GLint internalFormat, GLenum format, int filterType, int flipVertically);
unsigned int textureHandler_loadSkybox();

int textureHandler_importTextures(int stage)
{
    int problem = 0;
    
    switch (stage)
    {
    case TEXTURE_IN_GAME:
        atlas_albedo = textureHandler_loadImage("../assets/textures/atlas_albedo.png", GL_SRGB_ALPHA, GL_RGBA, GL_NEAREST, 69);
        if (atlas_albedo == 0)
            problem++;

        atlas_specular = textureHandler_loadImage("../assets/textures/atlas_specular.png", GL_RGB, GL_RGBA, GL_NEAREST, 69);
        if (atlas_specular == 0)
            problem++;

        atlas_normal = textureHandler_loadImage("../assets/textures/atlas_normal.png", GL_RGBA, GL_RGBA, GL_NEAREST, 69);
        if (atlas_normal == 0)
            problem++;



        water_normal = textureHandler_loadImage("../assets/textures/walter/walter_normal.png", GL_RGBA, GL_RGBA, GL_LINEAR, 69);
        if (water_normal == 0)
            problem++;

        water_dudv = textureHandler_loadImage("../assets/textures/walter/walter_dudv.png", GL_RGBA, GL_RGBA, GL_LINEAR, 69);
        if (water_dudv == 0)
            problem++;



        sky_gradient = textureHandler_loadImage("../assets/textures/sky/sky_gradient.png", GL_RGBA, GL_RGBA, GL_LINEAR, 0);
        glBindTexture(GL_TEXTURE_2D, sky_gradient);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        if (sky_gradient == 0)
            problem++;
        break;

    case TEXTURE_MAIN_MENU:
        menu_background = textureHandler_loadImage("../assets/textures/main_menu/main_bg.png", GL_RGB, GL_RGBA, GL_NEAREST, 69);
        if (menu_background == 0)
            problem++;

        menu_title = textureHandler_loadImage("../assets/textures/main_menu/main_title.png", GL_RGBA, GL_RGBA, GL_NEAREST, 69);
        if (menu_title == 0)
            problem++;
        break;
    }

    return problem;
}

void textureHandler_destroyTextures(int stage)
{
    switch (stage)
    {
    case TEXTURE_IN_GAME:
        glDeleteTextures(1, &atlas_albedo);
        glDeleteTextures(1, &atlas_specular);
        glDeleteTextures(1, &atlas_normal);

        glDeleteTextures(1, &water_dudv);
        glDeleteTextures(1, &water_normal);

        glDeleteTextures(1, &sky_gradient);

        atlas_albedo = 0;
        atlas_specular = 0;
        atlas_normal = 0;

        water_dudv = 0;
        water_normal = 0;

        sky_gradient = 0;
        break;

    case TEXTURE_MAIN_MENU:
        glDeleteTextures(1, &menu_background);
        glDeleteTextures(1, &menu_title);

        menu_background = 0;
        menu_title = 0;
        break;
    }
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

    case TEXTURE_WATER_NORMAL:
        return water_normal;

    case TEXTURE_WATER_DUDV:
        return water_dudv;

    case TEXTURE_SKY_GRADIENT:
        return sky_gradient;

    case TEXTURE_MENU_BACKGROUND:
        return menu_background;

    case TEXTURE_MENU_TITLE:
        return menu_title;

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