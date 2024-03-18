#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <miniaudio.h>
#include <pthread.h>

#include "../utils/seqtor.h"

//path to menu sounds
#define AUDIO_MUSIC_MENU_1_PATH "../assets/audio/music/music_menu_1.mp3"

//path to ingame sounds
#define AUDIO_SFX_GAME_JOIN_PATH "../assets/audio/sfx/game_join.mp3"
#define AUDIO_SFX_JUMP_PATH "../assets/audio/sfx/jump.mp3"
#define AUDIO_SFX_BLOCK_BREAK_PATH "../assets/audio/sfx/block_break.mp3"
#define AUDIO_SFX_BLOCK_PLACE_PATH "../assets/audio/sfx/block_place.mp3"

//path to miscellaneous sounds


//---------------------------------------

struct sound {
	ma_sound* data;
	sound_id_t id;
};


static sound_id_t currentId = 1;

static ma_engine* engine=NULL;
static pthread_mutex_t* mutex = NULL;
static int currentEngineState = AUDIO_NONE;

//sound initializers
static ma_sound sfx_game_join;
static ma_sound sfx_jump;
static ma_sound sfx_block_break;
static ma_sound sfx_block_place;

static ma_sound music_menu_1;

//active sounds
static sound activeSounds[AUDIO_MAX_COUNT_OF_SIMULTANEOUS_SOUNDS];

void audio_loadSounds(int);
void audio_unloadSounds(int);

//returns 0 if problemlos
int audio_init(int engineState) 
{
	if (engine != NULL)
		return;

	ma_result result;
	
	engine = malloc(sizeof(ma_engine));
	mutex = malloc(sizeof(pthread_mutex_t));

	pthread_mutex_init(mutex, NULL);
	pthread_mutex_lock(mutex);

	result = ma_engine_init(NULL, engine);
	if (result != MA_SUCCESS) {
		printf("Failed to initialize audio engine.");
		free(engine);
		engine = NULL;

		pthread_mutex_unlock(mutex);
		free(mutex);
		mutex = NULL;
		return -1;
	}

	currentEngineState = engineState;
	audio_loadSounds(currentEngineState);

	for (unsigned int i = 0; i < AUDIO_MAX_COUNT_OF_SIMULTANEOUS_SOUNDS; i++)
	{
		activeSounds[i].data = NULL;
		activeSounds[i].id = 0;//0 means unused
	}
	
	pthread_mutex_unlock(mutex);
	return 0;
}

void audio_destroy()
{
	if (engine == NULL)
		return;

	pthread_mutex_lock(mutex);

	for (unsigned int i = 0; i < AUDIO_MAX_COUNT_OF_SIMULTANEOUS_SOUNDS; i++)
	{
		if (activeSounds[i].id == 0)
			continue;

		ma_sound_uninit(activeSounds[i].data);
		free(activeSounds[i].data);
		activeSounds[i].id = 0;//0 means unused
	}

	audio_unloadSounds(currentEngineState);
	ma_engine_uninit(engine);
	free(engine);
	engine = NULL;
	currentEngineState = 0;

	pthread_mutex_unlock(mutex);
	pthread_mutex_destroy(mutex);
	free(mutex);
	mutex = NULL;
}

void audio_cleanupUnused()
{
	if (engine == NULL)
		return;

	pthread_mutex_lock(mutex);

	for (int i = 0; i < AUDIO_MAX_COUNT_OF_SIMULTANEOUS_SOUNDS; i++)
	{
		if (activeSounds[i].id != 0 && ma_sound_at_end(activeSounds[i].data) != 0)
		{
			ma_sound_uninit(activeSounds[i].data);
			free(activeSounds[i].data);
			activeSounds[i].id = 0;
		}
	}

	pthread_mutex_unlock(mutex);
}

sound_id_t audio_playSound(int _sound)
{
	if (engine == NULL)
	{
		return 0;
	}

	pthread_mutex_lock(mutex);

	sound s;
	s.data = malloc(sizeof(ma_sound));

	ma_result result=MA_SUCCESS;

	switch (_sound)//nem baj, hogy itt init_from_file-okat hiv, mert az audio mar be lett toltve egyszer, es ezt a miniaudio tudja, szoval nem tolti be megegyszer (aszem)
	{
		case AUDIO_SFX_GAME_JOIN:
			result = ma_sound_init_from_file(engine, AUDIO_SFX_GAME_JOIN_PATH, 0, NULL, NULL, s.data);
			break;

		case AUDIO_SFX_JUMP:
			result = ma_sound_init_from_file(engine, AUDIO_SFX_JUMP_PATH, 0, NULL, NULL, s.data);
			break;

		case AUDIO_SFX_BLOCK_BREAK:
			result = ma_sound_init_from_file(engine, AUDIO_SFX_BLOCK_BREAK_PATH, 0, NULL, NULL, s.data);
			break;

		case AUDIO_SFX_BLOCK_PLACE:
			result = ma_sound_init_from_file(engine, AUDIO_SFX_BLOCK_PLACE_PATH, 0, NULL, NULL, s.data);
			break;

		case AUDIO_MUSIC_MENU_1:
			result = ma_sound_init_from_file(engine, AUDIO_MUSIC_MENU_1_PATH, 0, NULL, NULL, s.data);
			break;

		default:
			free(s.data);
			pthread_mutex_unlock(mutex);
			return 0;
	}

	if (result != MA_SUCCESS)
	{
		pthread_mutex_unlock(mutex);
		return 0;
	}

	//actually play the sound
	int index = -1;
	for (int i = 0; i < AUDIO_MAX_COUNT_OF_SIMULTANEOUS_SOUNDS; i++)//search for unused sounds
	{
		if (activeSounds[i].id==0)
		{
			index = i;
			break;
		}
	}

	for (int i = 0; i < AUDIO_MAX_COUNT_OF_SIMULTANEOUS_SOUNDS; i++)//search for not freed but finished sounds
	{
		if (activeSounds[i].id != 0&&ma_sound_at_end(activeSounds[i].data)!=0)
		{
			ma_sound_uninit(activeSounds[i].data);
			free(activeSounds[i].data);
			index = i;
			break;
		}
	}

	if (index == -1)//no place for a new sound
	{
		free(s.data);
		pthread_mutex_unlock(mutex);
		return 0;
	}

	result = ma_sound_start(s.data);
	if (result != MA_SUCCESS)
	{
		free(s.data);
		pthread_mutex_unlock(mutex);
		return 0;
	}

	//add sound to the activeSounds
	s.id = currentId++;
	activeSounds[index] = s;

	pthread_mutex_unlock(mutex);

	return s.id;
}

int audio_soundAtEnd(sound_id_t soundId)
{
	if (engine == NULL || soundId == 0)
		return 69;

	pthread_mutex_lock(mutex);

	int index = -1;
	for (int i = 0; i < AUDIO_MAX_COUNT_OF_SIMULTANEOUS_SOUNDS; i++)
	{
		if (activeSounds[i].id == soundId)
		{
			index = i;
			break;
		}
	}

	if (index == -1 || ma_sound_at_end(activeSounds[index].data) != 0)
	{
		pthread_mutex_unlock(mutex);
		return 69;
	}

	pthread_mutex_unlock(mutex);
	return 0;
}

void audio_stopSound(sound_id_t soundId)
{
	if (engine == NULL || soundId == 0)
		return;

	pthread_mutex_lock(mutex);

	int index = -1;
	for (int i = 0; i < AUDIO_MAX_COUNT_OF_SIMULTANEOUS_SOUNDS; i++)
	{
		if (activeSounds[i].id == soundId)
		{
			index = i;
			break;
		}
	}

	if (index != -1)
	{
		ma_sound_stop(activeSounds[index].data);
		free(activeSounds[index].data);
		activeSounds[index].data = NULL;
		activeSounds[index].id = 0;
	}

	pthread_mutex_unlock(mutex);
}

//ide nem szabad mutexet rakni, mert mutexes fuggvenybol van meghivva
void audio_loadSounds(int currentState)//currentState erteke AUDIO_INGAME vagy AUDIO_MENU
{
	if (engine == NULL)
		return;


	switch (currentState)
	{
	case AUDIO_INGAME:
		ma_sound_init_from_file(engine, AUDIO_SFX_GAME_JOIN_PATH, 0, NULL, NULL, &sfx_game_join);
		ma_sound_init_from_file(engine, AUDIO_SFX_JUMP_PATH, 0, NULL, NULL, &sfx_jump);
		ma_sound_init_from_file(engine, AUDIO_SFX_BLOCK_BREAK_PATH, 0, NULL, NULL, &sfx_block_break);
		ma_sound_init_from_file(engine, AUDIO_SFX_BLOCK_PLACE_PATH, 0, NULL, NULL, &sfx_block_place);
		break;

	case AUDIO_MENU:
		ma_sound_init_from_file(engine, AUDIO_MUSIC_MENU_1_PATH, 0, NULL, NULL, &music_menu_1);
		break;
	}
}

void audio_unloadSounds(int currentState)
{
	if (engine == NULL)
		return;


	switch (currentState)
	{
	case AUDIO_INGAME:
		ma_sound_uninit(&sfx_game_join);
		ma_sound_uninit(&sfx_jump);
		ma_sound_uninit(&sfx_block_break);
		ma_sound_uninit(&sfx_block_place);
		break;

	case AUDIO_MENU:
		ma_sound_uninit(&music_menu_1);
		break;
	}
}