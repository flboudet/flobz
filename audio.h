#ifndef _AUDIO_H
#define _AUDIO_H

/// AUDIO

#ifdef USE_AUDIO
#include <SDL/SDL_mixer.h>
typedef Mix_Chunk Sound;
#else
#define Sound void
#endif

/// les differents sons utilisables
extern Sound *sound_pop;
extern Sound *sound_bam1;
extern Sound *sound_fff;
extern Sound *sound_slide;
#define NB_YAHOOHOO1 4
#define NB_YAHOOHOO2 2
#define NB_YAHOOHOO3 1
extern Sound *sound_yahoohoo1[NB_YAHOOHOO1];
extern Sound *sound_yahoohoo2[NB_YAHOOHOO2];
extern Sound *sound_yahoohoo3[NB_YAHOOHOO3];
extern Sound *sound_splash[8];

void    audio_init ();
void    audio_music_start (int num);
void    audio_sound_play (Sound * s);
void    audio_close ();

/// volume compris entre 0 et 100;
void    audio_music_set_volume (int vol);

/// volume compris entre 0 et 100;
void    audio_set_volume (int vol);

void    audio_set_music_on_off (int on);
void    audio_set_sound_on_off (int on);


#define NB_MUSIC_THEME 2
void    audio_music_switch_theme(int theme_number);

#endif
