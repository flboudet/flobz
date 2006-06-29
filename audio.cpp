#include <string.h>
#include "audio.h"
#include "preferences.h"

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
#include "PuyoCommander.h"

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

static const char * kMusicVolume = "AudioManager.Music.Volume";
static const char * kSoundVolume = "AudioManager.FX.Volume";
static const char * kMusic       = "AudioManager.Music.State";
static const char * kSound       = "AudioManager.FX.State";


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

void AudioManager::init()
{

#ifdef USE_AUDIO
    int i;

    audio_supported = (Mix_OpenAudio (MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) == 0);
    if (!audio_supported) return;

    Mix_QuerySpec (&audio_rate, &audio_format, &audio_channels);

    music_on = GetBoolPreference(kMusic,true);
    sound_on = GetBoolPreference(kSound,true);

    music_volume = ((float)GetIntPreference(kMusicVolume, 100))/100.0f;
    sound_volume = ((float)GetIntPreference(kSoundVolume, 100))/100.0f;

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
    
    String filePath = theCommander->getDataPathManager().getPath(FilePath("sfx").combine(fileName));
    Mix_Chunk *result;

    result = Mix_LoadWAV(filePath);
    if (result)
      Mix_VolumeChunk (result, volume);
    return result;
}

static Mix_Music *CustomMix_LoadMUS(const char *fileName)
{
    if (!audio_supported) return NULL;

    String filePath = theCommander->getDataPathManager().getPath(FilePath("music").combine(fileName));
    Mix_Music *result;
    
    result = Mix_LoadMUS(fileName);
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
  if ((!audio_supported) || (music_current == fileName)) return;
    
    music_current = fileName;

    if (music_on)
    {
      preloadMusic(fileName);
      Mix_Music *music = musicCache.get(fileName);
      Mix_HaltMusic();
      if (music != NULL) Mix_PlayMusic(music, -1);
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

    SetIntPreference(kMusicVolume, (int)(volume*100.0f));
#endif
}

void AudioManager::soundVolume(float volume)
{
#ifdef USE_AUDIO
    if (!audio_supported) return;

    if (sound_on)
    {
        Mix_Volume (-1, (int) ((float)MIX_MAX_VOLUME * volume));
    }
    sound_volume = volume;

    SetIntPreference(kSoundVolume, (int)(volume*100.0f));
#endif
}

void AudioManager::musicOnOff(bool state)
{
#ifdef USE_AUDIO
  if ((!audio_supported) || (music_on == state)) return;

  music_on = state;

  if (music_on)
  {
    musicVolume(music_volume);
    int lenght = music_current.size();
    char tmp[lenght+1];
    music_current.copy(tmp,lenght,0);
    playMusic(tmp);
  }
  else
  {
    Mix_HaltMusic();
  }
  SetBoolPreference(kMusic, music_on);
#endif
}

void AudioManager::soundOnOff(bool state)
{
#ifdef USE_AUDIO
  if ((!audio_supported) || (sound_on == state)) return;
  
  sound_on = state;
  SetBoolPreference(kSound, sound_on);
#endif
}

bool AudioManager::isMusicOn()
{
  return music_on;
}

bool AudioManager::isSoundOn()
{
  return sound_on;
}
