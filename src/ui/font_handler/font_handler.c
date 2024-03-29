#include "font_handler.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

static void _fontHandler_loadGlyphs(font* f, FT_Face face);

static FT_Library _ft;

static int isInitialized = 0;

int fontHandler_init()
{
    if (isInitialized)
        return 0;

    if (FT_Init_FreeType(&_ft))
    {
        printf("ERROR::FREETYPE: Could not init FreeType Library\n");
        return 69;
    }
    isInitialized = 69;
    return 0;
}
void fontHandler_close()
{
    if (isInitialized == 0)
        return;

    FT_Done_FreeType(_ft);
    isInitialized = 0;
}

int fontHandler_isInitialized()
{
    return isInitialized;
}

font fontHandler_loadFont(const char* fontPath, unsigned int fontSize)
{
    FT_Face face;
    if (FT_New_Face(_ft, fontPath, 0, &face))
    {
        printf("ERROR::FREETYPE: Failed to load font\n");
        return (font){ 0 };
    }
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
    
    font f;
    f.fontSize = fontSize;
    _fontHandler_loadGlyphs(&f, face);
    FT_Done_Face(face);
    return f;
}

static void _fontHandler_loadGlyphs(font* f, FT_Face face)
{
    unsigned int lineHeight = 0;
    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyphs
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            printf("ERROR::FREETYTPE: Failed to load Glyph\n");
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        character ch = {
            texture, 
            face->glyph->bitmap.width, face->glyph->bitmap.rows,
            face->glyph->bitmap_left, face->glyph->bitmap_top,
            face->glyph->advance.x
        };
        f->characters[c] = ch;

        if (ch.height > lineHeight)
            lineHeight = ch.height;
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    f->lineHeight = lineHeight;
}

void fontHandler_destroyFont(font* f)
{
    for (unsigned char c = 0; c < 128; c++)
    {
        glDeleteTextures(1, &f->characters->textureID);
    }
}

float fontHandler_calculateTextLength(font* f, const char* text)
{
    float length = 0;
    // iterate through all characters
    for (int i = 0; text[i] != '\0'; i++)
    {
        character ch = f->characters[text[i]];
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        length += ch.advance >> 6; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    return length;
}