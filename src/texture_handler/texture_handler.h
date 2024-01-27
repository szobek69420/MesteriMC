#ifndef TEXTURE_HANDLER_H
#define TEXTURE_HANDLER_H

//textures in project

#define TEXTURE_ATLAS_ALBEDO 1
#define TEXTURE_ATLAS_SPECULAR 2
#define TEXTURE_ATLAS_NORMAL 3

#define TEXTURE_WATER_NORMAL 69
#define TEXTURE_WATER_DUDV 70

#define TEXTURE_SKYBOX 69420//unused
#define TEXTURE_SUN 69421//unused
#define TEXTURE_SKY_GRADIENT 69422

int textureHandler_importTextures();//visszateresi ertek 0, ha sikerult
void textureHandler_destroyTextures();

unsigned int textureHandler_getTexture(int texture);

unsigned int textureHandler_loadImage(const char* pathToTexture, GLint internalFormat, GLenum format, int filterType, int flipVertically);

#endif