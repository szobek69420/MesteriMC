#ifndef AUDIO_H
#define AUDIO_H

#define AUDIO_MAX_COUNT_OF_SIMULTANEOUS_SOUNDS 100

#define AUDIO_NONE 642
#define AUDIO_INGAME 666
#define AUDIO_MENU 669

//menu sfx and music
#define AUDIO_MUSIC_MENU_1 42069

//ingame sfx and music
#define AUDIO_SFX_GAME_JOIN 69420
#define AUDIO_SFX_JUMP 69421
#define AUDIO_SFX_BLOCK_BREAK 69422
#define AUDIO_SFX_BLOCK_PLACE 69423

#define AUDIO_CAVE_NOISE_COUNT 8
#define AUDIO_SFX_CAVE_NOISE_1 69666
#define AUDIO_SFX_CAVE_NOISE_2 69667
#define AUDIO_SFX_CAVE_NOISE_3 69668
#define AUDIO_SFX_CAVE_NOISE_4 69669
#define AUDIO_SFX_CAVE_NOISE_5 69670
#define AUDIO_SFX_CAVE_NOISE_6 69671
#define AUDIO_SFX_CAVE_NOISE_7 69672
#define AUDIO_SFX_CAVE_NOISE_8 69673

//both menu and ingame sfx and music

//-------------------------------

struct sound;
typedef struct sound sound;

typedef unsigned int sound_id_t;

//returns 0 if problemlos (the value of the engineState is either AUDIO_INGAME or AUDIO_MENU
int audio_init(int engineState);
void audio_destroy();

//removes finished sounds from the list so that the response time of audio_playSound will always be immediate
void audio_cleanupUnused();

//returns the id of the sound (0 if it was unsuccessful)
sound_id_t audio_playSound(int sound);
void audio_stopSound(sound_id_t soundId);
int audio_soundAtEnd(sound_id_t s);

#define AUDIO_PLAY_RANDOM_CAVE_NOISE() audio_playSound(AUDIO_SFX_CAVE_NOISE_1+(int)(AUDIO_CAVE_NOISE_COUNT*((float)rand()/(float)RAND_MAX)))

#endif