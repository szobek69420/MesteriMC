#ifndef AUDIO_H
#define AUDIO_H


#define AUDIO_NONE 642
#define AUDIO_INGAME 666
#define AUDIO_MENU 669

//menu sfx and music

//ingame sfx and music
#define AUDIO_SFX_GAME_JOIN 69420
#define AUDIO_SFX_JUMP 69421
#define AUDIO_SFX_BLOCK_BREAK 69422
#define AUDIO_SFX_BLOCK_PLACE 69423

//both menu and ingame sfx and music

//-------------------------------

enum audio_play_status{
	AUDIO_PLAY_SUCCESSFUL,
	AUDIO_PLAY_ERROR
};

//sounds cannot be reused!!!
typedef struct sound {
	const void * data;
	enum audio_play_status status;
} sound;

//returns 0 if problemlos (the value of the engineState is either AUDIO_INGAME or AUDIO_MENU
int audio_init(int engineState);
void audio_destroy();

sound audio_playSound(int sound);
void audio_stopSound(sound* s);
int audio_soundAtEnd(sound* s);

#endif