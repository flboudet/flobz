#include <string.h>
#include "audio.h"

extern char *dataFolder;

Sound  *sound_pop;
Sound  *sound_slide;
Sound  *sound_bam1;
Sound  *sound_fff;
Sound  *sound_yahoohoo1[NB_YAHOOHOO1];
Sound  *sound_yahoohoo2[NB_YAHOOHOO2];
Sound  *sound_yahoohoo3[NB_YAHOOHOO3];
Sound  *sound_splash[8];
Sound  *sound_bim[2];


#ifdef USE_AUDIO
#include "glSDL.h"
static Mix_Music *music[4];

static char *MUSIC_THEME[2][4] = {
  { "flobopuyo_menu.xm", "flobopuyo_game1.xm", "flobopuyo_game2.xm", "flobopuyo_gameover.xm" },
  { "flobopuyo_menu.xm", "strange_fear.xm", "strange_fear2.xm", "strange_gameover.xm" }
};

static int sound_supported;
static int volume;
static int sound_on;
static int music_on;
static int currentMus = -1;

static Sound *CustomMix_LoadWAV(char *path, char *fileName, int volume)
{
    Sound *result;
    char temp[1024];
    if (!sound_supported) return NULL;
    sprintf(temp, "%s/sfx/%s", dataFolder, fileName);
    result = Mix_LoadWAV(temp);
    if (result)
      Mix_VolumeChunk (result, volume);
    return result;
}

static Mix_Music *CustomMix_LoadMUS(char *path, char *fileName)
{
    Mix_Music *result;
    char temp[1024];
    if (!sound_supported) return NULL;
    sprintf(temp, "%s/sfx/%s", dataFolder, fileName);
    result = Mix_LoadMUS(temp);
    return result;
}
#endif

void
audio_init ()
{
#ifdef USE_AUDIO
	int     audio_rate;
	Uint16  audio_format;
	int     audio_channels;
  int i;

	sound_supported =
    (Mix_OpenAudio (MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 256) == 0);

  if (!sound_supported) return;

	sound_pop   = CustomMix_LoadWAV (dataFolder, "pop.wav", 30);
	sound_bam1  = CustomMix_LoadWAV (dataFolder, "bam1.wav", 12);
	sound_fff   = CustomMix_LoadWAV (dataFolder, "fff.wav", 48);
	sound_slide = CustomMix_LoadWAV (dataFolder, "slide.wav", 128);

  sound_yahoohoo1[0] = NULL;
  sound_yahoohoo1[1] = CustomMix_LoadWAV (dataFolder, "yahoohoo.wav", 50);
  sound_yahoohoo1[2] = CustomMix_LoadWAV (dataFolder, "woho.wav", 32);
  sound_yahoohoo1[3] = CustomMix_LoadWAV (dataFolder, "pastaga.wav", 70);

  sound_yahoohoo2[0] = sound_yahoohoo1[2];
  sound_yahoohoo2[1] = CustomMix_LoadWAV (dataFolder, "woo.wav", 45);
  
  sound_yahoohoo3[0] = CustomMix_LoadWAV (dataFolder, "applose.wav", 96);

  sound_splash[0] = CustomMix_LoadWAV (dataFolder, "splash1.wav",72);
	sound_splash[1] = CustomMix_LoadWAV (dataFolder, "splash2.wav",72);
	sound_splash[2] = CustomMix_LoadWAV (dataFolder, "splash3.wav",72);
	sound_splash[3] = CustomMix_LoadWAV (dataFolder, "splash4.wav",72);
	sound_splash[4] = CustomMix_LoadWAV (dataFolder, "splash5.wav",72);
	sound_splash[5] = CustomMix_LoadWAV (dataFolder, "splash6.wav",72);
	sound_splash[6] = CustomMix_LoadWAV (dataFolder, "splash7.wav",72);
	sound_splash[7] = CustomMix_LoadWAV (dataFolder, "splash8.wav",72);
    
    sound_bim[0] = CustomMix_LoadWAV (dataFolder, "bim1.wav",72);
    sound_bim[1] = CustomMix_LoadWAV (dataFolder, "bim2.wav",72);

  music[0] = CustomMix_LoadMUS (dataFolder, "flobopuyo_menu.xm");
	music[1] = CustomMix_LoadMUS (dataFolder, "flobopuyo_game1.xm");
	music[2] = CustomMix_LoadMUS (dataFolder, "flobopuyo_game2.xm");
	music[3] = CustomMix_LoadMUS (dataFolder, "flobopuyo_gameover.xm");

	Mix_QuerySpec (&audio_rate, &audio_format, &audio_channels);

	sound_on = 1;
	music_on = 1;
	audio_music_set_volume (100);
	audio_set_volume (50);
#endif
}

void
audio_music_start (int num)
{
#ifdef USE_AUDIO
  if (!sound_supported) return;
  if ((currentMus != num) && (music_on==true)) {
    currentMus = num;
    Mix_HaltMusic ();
    if (music[num])
      Mix_PlayMusic (music[num], -1);
  }
//	audio_music_set_volume (50);
#endif
}

void
audio_sound_play (Sound * s)
{
#ifdef USE_AUDIO
  if (!sound_supported) return;
  if (s)
  	Mix_PlayChannel (-1, s, 0);
#endif
}

void
audio_close ()
{
#ifdef USE_AUDIO
  if (!sound_supported) return;
	Mix_CloseAudio ();
#endif
}

// vol compris entre 0 et 100;
void
audio_music_set_volume (int vol)
{
#ifdef USE_AUDIO
  if (!sound_supported) return;
	Mix_VolumeMusic ((int) (128.0 * (double) vol / 100.0));
#endif
}

// vol compris entre 0 et 100;
void
audio_set_volume (int vol)
{
#ifdef USE_AUDIO
  if (!sound_supported) return;
	if (sound_on) {
		volume = Mix_Volume (-1, (int) (128.0 * (double) vol / 100.0));
	}
	else {
		volume = (int) (128.0 * (double) vol / 100.0);
	}
#endif
}

void
audio_set_music_on_off (int on)
{
#ifdef USE_AUDIO
  if (!sound_supported) return;
    if (on) {
        if (Mix_PausedMusic()) Mix_ResumeMusic();
        else
        {
            while (Mix_FadingMusic() == MIX_FADING_OUT) SDL_Delay(100);
        }
        if (music[currentMus])
            Mix_FadeInMusic (music[currentMus], -1, 1000);
    }
    else {
        while (Mix_FadingMusic() == MIX_FADING_IN) SDL_Delay(100);
        Mix_FadeOutMusic (1000);
    }
    music_on = on;
#endif
}

void
audio_set_sound_on_off (int on)
{
#ifdef USE_AUDIO
  if (!sound_supported) return;
	if (on) {
		Mix_Volume (-1, volume);
	}
	else {
		volume = Mix_Volume (-1, -1);
		Mix_Volume (-1, 0);
	}
	sound_on = on;
#endif
}

void    audio_music_switch_theme(int theme_number)
{
#ifdef USE_AUDIO
  int i;
  if (!sound_supported) return;
  
  Mix_Music * themusic[4];
  
  for (i=0; i<4; ++i) {
      themusic[i] = music[i];
      music[i] = CustomMix_LoadMUS (dataFolder, MUSIC_THEME[theme_number][i]);
  }

  Mix_HaltMusic();
  
  for (i=0; i<4; ++i) {
    Mix_FreeMusic(themusic[i]);
  }

 // Mix_PlayMusic();

#endif
}
