#ifndef SETTINGS_H
#define SETTINGS_H

#define SETTINGS_RENDER_DISTANCE 69420
#define SETTINGS_RENDERER_RESOLUTION 69421
#define SETTINGS_RENDERER_WIDTH 69422
#define SETTINGS_RENDERER_HEIGHT 69423
#define SETTINGS_SHADOWS 69424
#define SETTINGS_SHADOW_RESOLUTION 69425
#define SETTINGS_SHADOW_RESOLUTION_PIXELS 69426

void settings_load();
void settings_save();
int settings_getInt(int option);
void settings_setInt(int option, int value);

#endif