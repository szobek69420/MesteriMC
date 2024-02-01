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

struct canvasText;//stores all the information about a text component of the canvas
typedef struct canvasText canvasText;


canvas* canvas_create(int width, int height, const char* fontSauce);
void canvas_destroy(canvas* c);

void canvas_render(canvas* c);
void canvas_calculatePositions(canvas* c);

int canvas_addText(canvas* c, const char* text, int hAlign, int vAlign, int x, int y, float r, float g, float b, int fontSize);
void canvas_removeComponent(canvas* c, int id);

#endif