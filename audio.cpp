#include <string.h>
#include "audio.h"
#include "preferences.h"

#define TIMEMS_BETWEEN_SAME_SOUND 10.0

#ifdef USE_AUDIO
#include <SDL_mixer.h>
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
static bool music_starting = false;

static std::string music_current = "";
static std::string music_command = "";

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
        typedef std::map<std::string, double> LastUseMap;
        typedef std::map<std::string, T*>  TMap;

        NameVector   nameVector;
        TMap         tMap;
        LastUseMap   lastUseMap;

        double          curTime;
        int          maxT;

    public:
        Cache(int maxT) : curTime(0.0), maxT(maxT) {}

        bool contains(const std::string &c)
        {
            NameVector::iterator it = nameVector.begin();
            while (it != nameVector.end()) {
                if (*it == c) {
                    return true;
                }
                ++it;
            }
            return false;
        }

        /* Like get, but does not check if 'c' is in the cache.
         *
         * Make sure that "this->contains(c)" is true.
         */
        T* veryGet(const std::string &c)
        {
            lastUseMap[c] = ios_fc::getTimeMs();
            return tMap[c];
        }

        /* Return the time of the last use of 'c'.
         *
         * Make sure that "this->contains(c)" is true.
         */
        double veryGetLastUse(std::string &c)
        {
            return lastUseMap[c];
        }

        T* add(const std::string &c, T* t)
        {
            T* ret = NULL;
            if (nameVector.size() >= (unsigned int)maxT)
                ret = removeOne();

            nameVector.push_back(c);
            tMap[c] = t;
            curTime = ios_fc::getTimeMs();
            lastUseMap[c] = curTime - TIMEMS_BETWEEN_SAME_SOUND - 0.0001;

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
    audio_supported = (Mix_OpenAudio (MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) == 0);
    if (!audio_supported) return;

    Mix_QuerySpec (&audio_rate, &audio_format, &audio_channels);

    Mix_AllocateChannels(16);

    music_on = GetBoolPreference(kMusic,true);
    sound_on = GetBoolPreference(kSound,true);

    music_volume = ((float)GetIntPreference(kMusicVolume, 100))/100.0f;
    sound_volume = ((float)GetIntPreference(kSoundVolume, 100))/100.0f;

    if (music_on) loadMusic("pop.xm");
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

AudioManager::AudioManager()
{
#ifdef USE_AUDIO
    GlobalNotificationCenter.addListener(String(kMusicVolume), this);
    GlobalNotificationCenter.addListener(String(kSoundVolume), this);
    GlobalNotificationCenter.addListener(String(kMusic), this);
    GlobalNotificationCenter.addListener(String(kSound), this);
#endif
}

AudioManager::~AudioManager()
{
#ifdef USE_AUDIO
    GlobalNotificationCenter.removeListener(String(kMusicVolume), this);
    GlobalNotificationCenter.removeListener(String(kSoundVolume), this);
    GlobalNotificationCenter.removeListener(String(kMusic), this);
    GlobalNotificationCenter.removeListener(String(kSound), this);
#endif
}

void AudioManager::notificationOccured(String identifier, void * context)
{
#ifdef USE_AUDIO
    if (identifier == kMusicVolume) {
        // musicVolume((float)*(int *)context);
    } else
        if (identifier == kSoundVolume) {
            soundVolume((float)*(int *)context);
        } else
            if (identifier == kMusic) {
                musicOnOff(*(bool *)context);
            } else
                if (identifier == kSound) {
                    soundOnOff(*(bool *)context);
                }
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

    result = Mix_LoadMUS(filePath);
    if (!result)
        printf("Mix_LoadMUS(\"%s\"): %s\n", (const char *)filePath, Mix_GetError());
    return result;
}

void AudioManager::preloadMusic(const char *fileName)
{
#ifdef USE_AUDIO
    if (!audio_supported) return;

    if (musicCache.contains(fileName)) return;

    Mix_Music *music   = CustomMix_LoadMUS (fileName);
    Mix_Music *removed = musicCache.add(fileName, music);
    if (removed)
        Mix_FreeMusic(removed);
#endif
}

void AudioManager::loadMusic(const char *fileName)
{
#ifdef USE_AUDIO
  if ((!music_starting) && ((!audio_supported) || (music_current == fileName))) return;
    music_current = fileName;
    printf("Loading %s\n", fileName);

    if (music_on)
    {
      preloadMusic(fileName);
      Mix_Music *music = musicCache.veryGet(fileName);
      Mix_HaltMusic();
      if (music != NULL) Mix_PlayMusic(music, -1);
    }
#endif
}

void AudioManager::music(const char *command)
{
#ifdef USE_AUDIO
  static const int MSTART[5] = {1, 12, 15, 22, 34};
  static const int MGAME[4] = { MSTART[4], MSTART[3], MSTART[0], MSTART[2] };

  if (((!music_on) && (!audio_supported)) || (music_command == command)) return;
  music_command = command;
  if (music_command == "menu") {
      Mix_SetMusicPosition(MSTART[1]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.3));
  }
  else if (music_command == "credits") {
      Mix_SetMusicPosition(MSTART[0]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }
  else if (music_command == "network menu") {
      Mix_SetMusicPosition(MSTART[2]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.3));
  }
  else if (music_command == "level1") {
      Mix_SetMusicPosition(MGAME[0]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }
  else if (music_command == "level2") {
      Mix_SetMusicPosition(MGAME[1]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }
  else if (music_command == "level3") {
      Mix_SetMusicPosition(MGAME[2]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }
  else if (music_command == "level4") {
      Mix_SetMusicPosition(MGAME[3]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }
  else if (music_command == "level5") {
      Mix_SetMusicPosition(MGAME[0]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }
  else if (music_command == "level6") {
      Mix_SetMusicPosition(MGAME[1]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }
  else if (music_command == "level7") {
      Mix_SetMusicPosition(MGAME[2]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }
  else if (music_command == "level8") {
      Mix_SetMusicPosition(MGAME[3]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }
  else if (music_command == "level9") {
      Mix_SetMusicPosition(MGAME[0]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }
  /*else if (music_command == "game") { keep music of interlevel for now.
      Mix_SetMusicPosition(MSTART[rand()%4]);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }*/
  else if (music_command == "game won") {
      Mix_SetMusicPosition(15);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }
  else if (music_command == "game over") {
      Mix_SetMusicPosition(15);
      Mix_VolumeMusic ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }
#endif
}

void AudioManager::preloadSound(const char *fileName, float volume)
{
#ifdef USE_AUDIO
    if (!audio_supported) return;

    if (chunkCache.contains(fileName)) return;

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
    std::string c = fileName;
    if (ios_fc::getTimeMs() - chunkCache.veryGetLastUse(c) < TIMEMS_BETWEEN_SAME_SOUND)
        return;

    Mix_Chunk *chunk = chunkCache.veryGet(c);
    if (chunk != NULL) {
        int channel = Mix_PlayChannel (-1, chunk, 0);
        Uint8 leftVolume = (Uint8)(255.*((-balance + 1.)/2.));
        Mix_SetPanning(channel, leftVolume, 255-leftVolume);
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
    {
        Mix_Volume (-1, (int) ((float)MIX_MAX_VOLUME * volume));
    }
    sound_volume = volume;
#endif
}

const char * AudioManager::musicVolumeKey(void) { return kMusicVolume; }
const char * AudioManager::soundVolumeKey(void) { return kSoundVolume; }
const char * AudioManager::musicOnOffKey(void)  { return kMusic; }
const char * AudioManager::soundOnOffKey(void)  { return kSound; }

void AudioManager::musicOnOff(bool state)
{
#ifdef USE_AUDIO
  if ((!audio_supported) || (music_on == state)) return;

  music_on = state;

  if (music_on)
  {
    music_starting = true;
    musicVolume(music_volume);
    int lenght = music_current.size();
    char tmp[lenght+1];
    music_current.copy(tmp,lenght,0);
    tmp[lenght] = 0;
    loadMusic(tmp);
    music_starting = false;
  }
  else
  {
    Mix_HaltMusic();
  }
#endif
}

void AudioManager::soundOnOff(bool state)
{
#ifdef USE_AUDIO
  if ((!audio_supported) || (sound_on == state)) return;

  sound_on = state;
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
