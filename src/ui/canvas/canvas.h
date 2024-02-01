#ifndef CANVAS_H
#define CANVAS_H

//fuggoleges igazitas
#define CANVAS_ALIGN_TOP 0
#define CANVAS_ALIGN_MIDDLE 1
#define CANVAS_ALIGM_BOTTOM 2

//vizszintes igazitas
#define CANVAS_ALIGN_LEFT 0
#define CANVAS_ALIGN_CENTER 1
#define CANVAS_ALIGN_RIGHT 2


struct canvas;
typedef struct canvas canvas;

struct canvasText;//stores all the information about a text component of the canvas
typedef struct canvasText canvasText;

#endif