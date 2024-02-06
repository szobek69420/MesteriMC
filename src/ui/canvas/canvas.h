#ifndef CANVAS_H
#define CANVAS_H

//fuggoleges igazitas
#define CANVAS_ALIGN_TOP 0
#define CANVAS_ALIGN_MIDDLE 1
#define CANVAS_ALIGN_BOTTOM 2

//vizszintes igazitas
#define CANVAS_ALIGN_LEFT 0
#define CANVAS_ALIGN_CENTER 1
#define CANVAS_ALIGN_RIGHT 2



struct canvas;
typedef struct canvas canvas;


//should be called after fontHandler_init
canvas* canvas_create(int width, int height, const char* fontSauce);
//should be called before fontHandler_close
void canvas_destroy(canvas* c);

void canvas_setSize(canvas* c, int width, int height);

void canvas_render(canvas* c, int mouseX, int mouseY, int mousePressed);
void canvas_checkMouseInput(canvas* c, int mouseX, int mouseY, int mouseClicked);

void canvas_removeComponent(canvas* c, int id);
void canvas_setComponentPosition(canvas* c, int id, int x, int y);
void canvas_setComponentAlignment(canvas* c, int id, int hAlign, int vAlign);
void canvas_setComponentSize(canvas* c, int id, float width, float height);

int canvas_addText(canvas* c, const char* text, int hAlign, int vAlign, int x, int y, float r, float g, float b, int fontSize);

void canvas_setTextText(canvas* c, int id, const char* text);
void canvas_setTextColour(canvas* c, int id, float r, float g, float b);
void canvas_setTextFontSize(canvas* c, int id, int fontSize);

int canvas_addButton(canvas* c, int hAlign, int vAlign, int x, int y, float width, float height);
void canvas_setButtonFillColour(canvas* c, int id, float r, float g, float b);
void canvas_setButtonBorderColour(canvas* c, int id, float r, float g, float b);
void canvas_setButtonBackgroundTransparency(canvas* c, int id, int transparentBackground);
void canvas_setButtonBorder(canvas* c, int id, float borderWidth, float borderRadius);
void canvas_setButtonClicked(canvas* c, int id, void (*onClick)(void*), void* param);
void canvas_setButtonText(canvas* c, int id, const char* text, int fontSize, float r, float g, float b);


int canvas_addImage(canvas* c, int hAlign, int vAlign, int x, int y, float width, float height, unsigned int textureId);
void canvas_setImageTint(canvas* c, int id, float r, float g, float b);
void canvas_setImageTexture(canvas* c, int id, unsigned int textureId);

#endif