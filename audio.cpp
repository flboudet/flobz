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
static bool   audio_supported = false;
static int    audio_rate;
static Uint16 audio_format;
static int    audio_channels;

static float sound_volume   = 1.0f;
static float music_volume   = 1.0f;

static bool sound_on = false;
static bool music_on = false;

static std::string music_current = "";
#endif


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

    sound_on = true;
    music_on = true;
    musicVolume(music_volume);
    soundVolume(sound_volume);
#endif
}

void AudioManager::close()
{
#ifdef USE_AUDIO
    if (!audio_supported) return;
    Mix_CloseAudio();
    clearMusicCache();
    clearSoundCache();
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

    if (music_current == fileName) return;
    
    preloadMusic(fileName);
    Mix_Music *music = musicCache.get(fileName);
    if (music != NULL) {
        Mix_HaltMusic();
        Mix_PlayMusic(music, -1);
        music_current = fileName;
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
    if ((!audio_supported) || (!sound_on)) return;

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
    {
        Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * volume));
    }
    
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

void AudioManager::musicOnOff(bool state)
{
#ifdef USE_AUDIO
  if (!audio_supported) return;

  if (state)
  {
    Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume));
    if (Mix_PausedMusic()) Mix_ResumeMusic();
    else
    {
      while (Mix_FadingMusic() == MIX_FADING_OUT) SDL_Delay(100);
    }
    if (musicCache.get(music_current))
      Mix_FadeInMusic (musicCache.get(music_current), -1, 1000);
  }
  else {
    if (Mix_PlayingMusic())
    {
      while (Mix_FadingMusic() == MIX_FADING_IN) SDL_Delay(100);
      Mix_FadeOutMusic (1000);
    }
    else Mix_VolumeMusic (0);
  }
  music_on = state;

#endif
}

void AudioManager::soundOnOff(bool state)
{
#ifdef USE_AUDIO
  if (!audio_supported) return;
  
  if (state)
  {
		Mix_Volume (-1, (int) ((float)MIX_MAX_VOLUME * sound_volume));
  }
  else
  {
    Mix_Volume (-1, 0);
  }
  sound_on = state;
#endif
}
