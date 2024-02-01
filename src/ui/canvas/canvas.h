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

void canvas_render(canvas* c);

int canvas_addText(canvas* c, const char* text, int hAlign, int vAlign, int x, int y, float r, float g, float b, int fontSize);
void canvas_removeComponent(canvas* c, int id);

void canvas_setTextText(canvas* c, int id, const char* text);
void canvas_setTextColour(canvas* c, int id, float r, float g, float b);
void canvas_setTextFontSize(canvas* c, int id, int fontSize);

#endif