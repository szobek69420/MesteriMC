#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <miniaudio.h>

#include "../utils/seqtor.h"

//path to menu sounds

//path to ingame sounds
#define AUDIO_SFX_GAME_JOIN_PATH "../assets/audio/sfx/game_join.mp3"
#define AUDIO_SFX_JUMP_PATH "../assets/audio/sfx/jump.mp3"
#define AUDIO_SFX_BLOCK_BREAK_PATH "../assets/audio/sfx/block_break.mp3"
#define AUDIO_SFX_BLOCK_PLACE_PATH "../assets/audio/sfx/block_break.mp3"

//path to miscellaneous sounds


//---------------------------------------

static ma_engine* engine;
static int currentEngineState = AUDIO_NONE;

static ma_sound sfx_game_join;
static ma_sound sfx_jump;
static ma_sound sfx_block_break;
static ma_sound sfx_block_place;

void audio_loadSounds(int);
void audio_unloadSounds(int);

//returns 0 if problemlos
int audio_init(int engineState) 
{
	if (engine != NULL)
		return;

	ma_result result;
	
	engine = malloc(sizeof(ma_engine));

	result = ma_engine_init(NULL, engine);
	if (result != MA_SUCCESS) {
		printf("Failed to initialize audio engine.");
		free(engine);
		engine = NULL;
		return -1;
	}

	currentEngineState = engineState;
	audio_loadSounds(currentEngineState);

	return 0;
}

void audio_destroy()
{
	if (engine == NULL)
		return;

	audio_unloadSounds(currentEngineState);
	ma_engine_uninit(engine);
	free(engine);
	engine = NULL;
	currentEngineState = 0;
}

sound audio_playSound(int _sound)
{
	sound s;
	s.data = 0;
	s.status = AUDIO_PLAY_SUCCESSFUL;
	if (engine == NULL)
	{
		s.status = AUDIO_PLAY_ERROR;
		return s;
	}


	ma_result result;

	switch (_sound)//nem baj, hogy itt init_from_file-okat hiv, mert az audio mar be lett toltve egyszer, es ezt a miniaudio tudja, szoval nem tolti be megegyszer (aszem)
	{
		case AUDIO_SFX_GAME_JOIN:
			s.data = (const void*)&sfx_game_join;
			break;

		case AUDIO_SFX_JUMP:
			s.data = (const void*)&sfx_jump;
			break;

		case AUDIO_SFX_BLOCK_BREAK:
			s.data = (const void*)&sfx_block_break;
			break;

		case AUDIO_SFX_BLOCK_PLACE:
			s.data = (const void*)&sfx_block_place;
			break;

		default:
			s.status = AUDIO_PLAY_ERROR;
			return s;
	}

	if (s.status != AUDIO_PLAY_SUCCESSFUL)
		return s;

	//actually play the sound
	ma_sound_stop(s.data);
	result = ma_sound_start(s.data);
	if (result != MA_SUCCESS)
		s.status = AUDIO_PLAY_ERROR;

	return s;
}

int audio_soundAtEnd(sound* s)
{
	if (ma_sound_at_end((const ma_sound*)s->data) == 0)
		return 0;
	return 69;
}

void audio_stopSound(sound* s)
{
	if (ma_sound_at_end((const ma_sound*)s->data) != 0)
		return;
	ma_sound_stop(s);
}

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
		ma_sound_stop(&sfx_game_join);
		ma_sound_stop(&sfx_jump);
		ma_sound_stop(&sfx_block_break);
		ma_sound_stop(&sfx_block_place);

		ma_sound_uninit(&sfx_game_join);
		ma_sound_uninit(&sfx_jump);
		ma_sound_uninit(&sfx_block_break);
		ma_sound_uninit(&sfx_block_place);
		break;

	case AUDIO_MENU:

		break;
	}
}