#include "window.h"

static int window_width = 1000;
static int window_height = 800;
static float window_aspectXY = 1.25f;

int window_getWidth()
{
	return window_width;
}

int window_getHeight()
{
	return window_height;
}

void window_setWidth(int width)
{
	window_width = width;
	window_aspectXY = (float)window_width / window_height;
}

void window_setHeight(int height)
{
	window_height = height;
	window_aspectXY = (float)window_width / window_height;
}

float window_getAspect()
{
	return window_aspectXY;
}