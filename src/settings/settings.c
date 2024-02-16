#define _CRT_SECURE_NO_WARNINGS
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>

static const int renderResX[4] = { 640, 1280, 1920, 3840 };
static const int renderResY[4] = { 480, 720, 1080, 2160 };
static const int shadowRes[4] = { 1000, 2000, 4000, 6000 };

static int renderDistance = 0;
static int renderResolution = 0;
static int shadow = 0;
static int shadowResolution = 0;

static const int* settingsOrder[] = { &renderDistance, &renderResolution, &shadow, &shadowResolution };
static const char* settingsLabels[] = { "render distance: ", "resolution: ", "shadows: ", "shadow resolution: " };

void settings_load()
{
	FILE* file = fopen("../assets/config/open_me.ransomware", "r");
	if (file == NULL)
	{
		fputs("settings file was not found, you better recover it before i find you\n", stdin);
		return;
	}

	char c;
	for (int i = 0; i < sizeof(settingsOrder) / sizeof(int*); i++)
	{
		do {
			c = fgetc(file);
		} while (c != ':');
		c = fgetc(file);
		fscanf(file, "%d", settingsOrder[i]);
	}

	fclose(file);
}

void settings_save()
{
	FILE* file = fopen("../assets/config/open_me.ransomware", "w");
	if (file == NULL)
	{
		fputs("settings file could not be created, which is, indeed, suboptimal\n", stdin);
		return;
	}

	for (int i = 0; i < sizeof(settingsOrder) / sizeof(int*); i++)
	{
		fprintf(file, settingsLabels[i]);
		fprintf(file, "%d\n", *(settingsOrder[i]));
	}
	fclose(file);
}

int settings_getInt(int setting)
{
	switch (setting)
	{
	case SETTINGS_RENDER_DISTANCE:
		return renderDistance;

	case SETTINGS_RENDERER_WIDTH:
		return renderResX[renderResolution];

	case SETTINGS_RENDERER_HEIGHT:
		return renderResY[renderResolution];

	case SETTINGS_RENDERER_RESOLUTION:
		return renderResolution;

	case SETTINGS_SHADOWS:
		return shadow;

	case SETTINGS_SHADOW_RESOLUTION:
		return shadowResolution;

	case SETTINGS_SHADOW_RESOLUTION_PIXELS:
		return shadowRes[shadowResolution];
	}
}

void settings_setInt(int option, int value)
{
	switch (option)
	{
	case SETTINGS_RENDER_DISTANCE:
		renderDistance=value;
		break;

	case SETTINGS_RENDERER_RESOLUTION:
		if (value < 0)
			value = 0;
		if (value > sizeof(renderResX) / sizeof(renderResX[0]))
			value = sizeof(renderResX) / sizeof(renderResX[0]) - 1;
		renderResolution=value;
		break;

	case SETTINGS_SHADOWS:
		shadow = value;
		break;

	case SETTINGS_SHADOW_RESOLUTION:
		if (value < 0)
			value = 0;
		if (value > sizeof(shadowRes) / sizeof(shadowRes[0]))
			value = sizeof(shadowRes) / sizeof(shadowRes[0]) - 1;
		shadowResolution = value;
		break;
	}
}