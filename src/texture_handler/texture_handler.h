#ifndef TEXTURE_HANDLER_H
#define TEXTURE_HANDLER_H

//textures of stages
#define TEXTURE_IN_GAME 0
#define TEXTURE_MAIN_MENU 1

//textures in project

#define TEXTURE_ATLAS_ALBEDO 1
#define TEXTURE_ATLAS_ALBEDO_NON_SRGB 2
#define TEXTURE_ATLAS_SPECULAR 3
#define TEXTURE_ATLAS_SPECULAR_REDUCED 4
#define TEXTURE_ATLAS_NORMAL 5
#define TEXTURE_ATLAS_UI 6

#define TEXTURE_WATER_NORMAL 69
#define TEXTURE_WATER_DUDV 70

#define TEXTURE_SKYBOX 69420//unused
#define TEXTURE_SUN 69421//unused
#define TEXTURE_SKY_GRADIENT 69422
#define TEXTURE_SKY_GRADIENT_COLOUR_0 69423
#define TEXTURE_SKY_GRADIENT_COLOUR_1 69424
#define TEXTURE_SKY_GRADIENT_HORIZON 69426

#define TEXTURE_MENU_BACKGROUND 69420666
#define TEXTURE_MENU_TITLE 69420667
#define TEXTURE_MENU_TITLE_SETTINGS 69420668
#define TEXTURE_MENU_TITLE_PAUSE 69420669

//random
#define TEXTURE_PLAIN_WHITE 1000
#define TEXTURE_GOKU 1001

int textureHandler_importTextures(int stage);//visszateresi ertek 0, ha sikerult
void textureHandler_destroyTextures(int stage);

unsigned int textureHandler_getTexture(int texture);

unsigned int textureHandler_loadImage(const char* pathToTexture, GLint internalFormat, GLenum format, int filterType, int flipVertically);

#endif