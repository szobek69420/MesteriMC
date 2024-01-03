#include "font_handler.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

static void _fontHandler_loadGlyphs(font* f, FT_Face face);

static FT_Library _ft;

int fontHandler_init()
{
    if (FT_Init_FreeType(&_ft))
    {
        printf("ERROR::FREETYPE: Could not init FreeType Library\n");
        return 69;
    }
    return 0;
}
void fontHandler_close()
{
    FT_Done_FreeType(_ft);
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
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}