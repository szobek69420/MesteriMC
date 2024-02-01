#ifndef FONT_HANDLER_H
#define FONT_HANDLER_H

#include <ft2build.h>
#include FT_FREETYPE_H

#define MAX_CHARACTERS 128

typedef struct {
    unsigned int textureID;
    int width, height;
    int bearingX, bearingY;
    unsigned int advance;
} character;

typedef struct {
    unsigned int fontSize;
    character characters[MAX_CHARACTERS];
    unsigned int lineHeight;//max({characters[i].height})
} font;

int fontHandler_init();
void fontHandler_close();

int fontHandler_isInitialized();

font fontHandler_loadFont(const char* fontPath, unsigned int fontSize);
void fontHandler_destroyFont(font* f);

float fontHandler_calculateTextLength(font* f, const char* text);

#endif