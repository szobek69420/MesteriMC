#ifndef TEXTURE_HANDLER_H
#define TEXTURE_HANDLER_H

//textures in project

#define TEXTURE_ATLAS_ALBEDO 1
#define TEXTURE_ATLAS_SPECULAR 2
#define TEXTURE_ATLAS_NORMAL 3

int textureHandler_importTextures();//visszateresi ertek 0, ha sikerult
void textureHandler_destroyTextures();

unsigned int textureHandler_getTexture(int texture);

#endif