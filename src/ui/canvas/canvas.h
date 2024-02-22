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

//szinek
#define CANVAS_COLOUR_PRIMARY_0_R 0.8
#define CANVAS_COLOUR_PRIMARY_0_G 0.15
#define CANVAS_COLOUR_PRIMARY_0_B 1.0

#define CANVAS_COLOUR_PRIMARY_1_R 0.31
#define CANVAS_COLOUR_PRIMARY_1_G 0.0
#define CANVAS_COLOUR_PRIMARY_1_B 0.89

#define CANVAS_COLOUR_ACCENT_0_R 1.0
#define CANVAS_COLOUR_ACCENT_0_G 0.85
#define CANVAS_COLOUR_ACCENT_0_B 0.0

#define CANVAS_COLOUR_PRIMARY_0 CANVAS_COLOUR_PRIMARY_0_R,CANVAS_COLOUR_PRIMARY_0_G,CANVAS_COLOUR_PRIMARY_0_B
#define CANVAS_COLOUR_PRIMARY_1 CANVAS_COLOUR_PRIMARY_1_R,CANVAS_COLOUR_PRIMARY_1_G,CANVAS_COLOUR_PRIMARY_1_B
#define CANVAS_COLOUR_ACCENT_0 CANVAS_COLOUR_ACCENT_0_R,CANVAS_COLOUR_ACCENT_0_G,CANVAS_COLOUR_ACCENT_0_B



struct canvas;
typedef struct canvas canvas;


//should be called after fontHandler_init
canvas* canvas_create(int width, int height, const char* fontSauce);
//should be called before fontHandler_close
void canvas_destroy(canvas* c);

void canvas_setSize(canvas* c, int width, int height);

void canvas_render(canvas* c, int glfwMouseX, int glfwMouseY, int mousePressed);
void canvas_checkMouseInput(canvas* c, int mouseX, int mouseY, int mouseDown, int mousePressed,  int mouseClicked);

float canvas_calculateTextLength(canvas* c, const char* text, int fontSize);
float canvas_getTextLineHeight(canvas* c, int fontSize);

void canvas_removeComponent(canvas* c, int id);
void canvas_setComponentPosition(canvas* c, int id, int x, int y);
void canvas_setComponentAlignment(canvas* c, int id, int hAlign, int vAlign);
void canvas_setComponentSize(canvas* c, int id, float width, float height);


//text
int canvas_addText(canvas* c, const char* text, int hAlign, int vAlign, int x, int y, float r, float g, float b, int fontSize);
void canvas_setTextText(canvas* c, int id, const char* text);
void canvas_setTextColour(canvas* c, int id, float r, float g, float b);
void canvas_setTextFontSize(canvas* c, int id, int fontSize);


//button
int canvas_addButton(canvas* c, int hAlign, int vAlign, int x, int y, float width, float height);
void canvas_setButtonFillColour(canvas* c, int id, float r, float g, float b);
void canvas_setButtonBorderColour(canvas* c, int id, float r, float g, float b);
void canvas_setButtonBackgroundTransparency(canvas* c, int id, int transparentBackground);
void canvas_setButtonBorder(canvas* c, int id, float borderWidth, float borderRadius);
void canvas_setButtonPressed(canvas* c, int id, void(*onPress)(void*), void* param);//when just pressed the button
void canvas_setButtonClicked(canvas* c, int id, void (*onClick)(void*), void* param);//when just released the button
void canvas_setButtonEnter(canvas* c, int id, void(*onEnter)(void*), void* param);//when the pointer entered the button
void canvas_setButtonExit(canvas* c, int id, void(*onExit)(void*), void* param);
void canvas_setButtonText(canvas* c, int id, const char* text, int fontSize, float r, float g, float b);


//image
int canvas_addImage(canvas* c, int hAlign, int vAlign, int x, int y, float width, float height, unsigned int textureId);
void canvas_setImageTint(canvas* c, int id, float r, float g, float b);
void canvas_setImageTexture(canvas* c, int id, unsigned int textureId);
void canvas_setImageUV(canvas* c, int id, float uvX, float uvY, float uvWidth, float uvHeight);
void canvas_setImageClicked(canvas* c, int id, void (*onClick)(void*), void* param);


//slider
int canvas_addSlider(canvas* c, int hAlign, int vAlign, int x, int y, float width, float height, float knobWidth, float knobHeight, int wholeNumbers);
void canvas_setSliderCallback(canvas* c, int id, void(*dragged)(float));
void canvas_setSliderValue(canvas* c, int id, float value);
void canvas_setSliderBounds(canvas* c, int id, float minValue, float maxValue);
void canvas_setSliderWholeNumbers(canvas* c, int id, int wholeNumbers);

void canvas_setSliderBackgroundTransparency(canvas* c, int id, int transparentBackground);
void canvas_setSliderBackgroundBorder(canvas* c, int id, float borderWidth, float borderRadius);
void canvas_setSliderBackgroundFillColour(canvas* c, int id, float r, float g, float b);
void canvas_setSliderBackgroundBorderColour(canvas* c, int id, float r, float g, float b);

void canvas_setSliderKnobTransparency(canvas* c, int id, int knobTransparentBackground);
void canvas_setSliderKnobBorder(canvas* c, int id, float borderWidth, float borderRadius);
void canvas_setSliderKnobFillColour(canvas* c, int id, float r, float g, float b);
void canvas_setSliderKnobBorderColour(canvas* c, int id, float r, float g, float b);
void canvas_setKnobWidth(canvas* c, int id, float knobWidth, float knobHeight);


//block mesh
int canvas_addBlockMesh(canvas* c, int hAlign, int vAlign, int blockType, int x, int y, float width, float height);

#endif