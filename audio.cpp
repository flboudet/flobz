#include <string.h>
#include "audio.h"

#ifdef USE_AUDIO
 #ifdef MACOSX
  #include <SDL_mixer.h>
 #else
  #include <SDL/SDL_mixer.h>
 #endif
#endif

#include <vector>
#include <map>
#include <string>

static std::string audio_data_folder;

#ifdef USE_AUDIO
#include "glSDL.h"
static int    audio_supported = 0;
static int    audio_rate;
static Uint16 audio_format;
static int    audio_channels;

static float sound_volume   = 0.5;
static float music_volume   = 1.0;

static int sound_on = 0;
static int music_on = 0;
#endif

/*

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
    (Mix_OpenAudio (MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == 0);

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
*/

template <typename T>
class Cache
{
    private:
        typedef std::vector<std::string>   NameVector;
        typedef std::map<std::string, int> LastUseMap;
        typedef std::map<std::string, T*>  TMap;

        NameVector   nameVector;
        TMap         tMap;
        LastUseMap   lastUseMap;
        
        int          curTime;
        int          maxT;

    public:
        Cache(int maxT) : curTime(0), maxT(maxT) {}

        T* get(const std::string &c) 
        {
            NameVector::iterator it = nameVector.begin();
            while (it != nameVector.end()) {
                if (*it == c) {
                    lastUseMap[c] = ++curTime;
                    return tMap[c];
                }
                ++it;
            }
            return NULL;
        }

        T* add(const std::string &c, T* t)
        {
            T* ret = NULL;
            if (nameVector.size() >= maxT)
                ret = removeOne();
            
            nameVector.push_back(c);
            tMap[c] = t;
            lastUseMap[c] = ++curTime;

            return ret;
        }

        T* removeOne()
        {
            NameVector::iterator oldest = nameVector.begin();
            NameVector::iterator it     = nameVector.begin();
            while (it != nameVector.end())
            {
                if (lastUseMap[*it] < lastUseMap[*oldest])
                    oldest = it;
                ++it;
            }
            
            T* t = tMap[*oldest];
            tMap.erase(*oldest);
            lastUseMap.erase(*oldest);
            nameVector.erase(oldest);
            return t;
        }

        bool empty() const { return nameVector.size() == 0; }
};

#ifdef USE_AUDIO
Cache<Mix_Chunk> chunkCache(64);
Cache<Mix_Music> musicCache(6);
#endif

void AudioManager::init(const char *dataFolder)
{
    audio_data_folder = dataFolder;

#ifdef USE_AUDIO
    int i;

    audio_supported = (Mix_OpenAudio (MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) == 0);
    if (!audio_supported) return;

    Mix_QuerySpec (&audio_rate, &audio_format, &audio_channels);

    sound_on = 1;
    music_on = 1;
    musicVolume(music_volume);
    soundVolume(sound_volume);
#endif
}

void AudioManager::close()
{
#ifdef USE_AUDIO
    if (!audio_supported) return;
    clearMusicCache();
    clearSoundCache();
	Mix_CloseAudio();
#endif
}

void AudioManager::clearSoundCache()
{
#ifdef USE_AUDIO
    if (!audio_supported) return;

    while (!chunkCache.empty())
    {
        Mix_Chunk *removed = chunkCache.removeOne();
        if (removed)
            Mix_FreeChunk(removed);
    }
#endif
}

void AudioManager::clearMusicCache()
{
#ifdef USE_AUDIO
    if (!audio_supported) return;

    while (!musicCache.empty())
    {
        Mix_Music *removed = musicCache.removeOne();
        if (removed)
            Mix_FreeMusic(removed);
    }
#endif
}

static Mix_Chunk *CustomMix_LoadWAV(const char *fileName, int volume)
{
    if (!audio_supported) return NULL;
    
    Mix_Chunk *result;
    char temp[1024];

    sprintf(temp, "%s/%s", audio_data_folder.c_str(), fileName);

    result = Mix_LoadWAV(temp);
    if (result)
      Mix_VolumeChunk (result, volume);
    return result;
}

static Mix_Music *CustomMix_LoadMUS(const char *fileName)
{
    if (!audio_supported) return NULL;

    Mix_Music *result;
    char temp[1024];

    sprintf(temp, "%s/%s", audio_data_folder.c_str(), fileName);
    result = Mix_LoadMUS(temp);
    return result;
}

void AudioManager::preloadMusic(const char *fileName)
{
#ifdef USE_AUDIO
    if (!audio_supported) return;

    if (musicCache.get(fileName) != NULL) return;

    Mix_Music *music   = CustomMix_LoadMUS (fileName);
    Mix_Music *removed = musicCache.add(fileName, music);
    if (removed)
        Mix_FreeMusic(removed);
#endif
}

void AudioManager::playMusic(const char *fileName)
{
#ifdef USE_AUDIO
    if (!audio_supported) return;

    preloadMusic(fileName);
    Mix_Music *music = musicCache.get(fileName);
    if (music != NULL) {
        Mix_HaltMusic();
        Mix_PlayMusic(music, -1);
    }
#endif
}

void AudioManager::preloadSound(const char *fileName, float volume)
{
#ifdef USE_AUDIO
    if (!audio_supported) return;

    if (chunkCache.get(fileName) != NULL) return;

    Mix_Chunk *chunk   = CustomMix_LoadWAV (fileName, (int)(volume * MIX_MAX_VOLUME));
    Mix_Chunk *removed = chunkCache.add(fileName, chunk);
    if (removed)
        Mix_FreeChunk(removed);
#endif
}

void AudioManager::playSound(const char *fileName, float volume, float balance)
{
#ifdef USE_AUDIO
    if (!audio_supported) return;

    preloadSound(fileName, volume);
    Mix_Chunk *chunk = chunkCache.get(fileName);
    if (chunk != NULL) {
        int channel = -1;
        if (balance < -0.5f) channel = 0;
        if (balance >  0.5f) channel = 1;
        
        Mix_PlayChannel (channel, chunk, 0);
    }
#endif
}

void AudioManager::musicVolume(float volume)
{
#ifdef USE_AUDIO
    if (!audio_supported) return;

    if (music_on)
        Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * volume));

    music_volume = volume;
#endif
}

void AudioManager::soundVolume(float volume)
{
#ifdef USE_AUDIO
    if (!audio_supported) return;

    if (sound_on)
        Mix_Volume (-1, (int) ((float)MIX_MAX_VOLUME * volume));
    
    sound_volume = volume;
#endif
}
