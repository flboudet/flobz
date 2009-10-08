#include <string.h>
#include "audio.h"
#include "preferences.h"

#define TIMEMS_BETWEEN_SAME_SOUND 10.0

#include <vector>
#include <map>
#include <string>

#include "PuyoCommander.h"

#ifdef USE_AUDIO
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

Cache<audio_manager::Sound> chunkCache(64);
Cache<audio_manager::Music> musicCache(6);


audio_manager::AudioManager * AudioManager::m_audioManager;

void AudioManager::init()
{

    m_audioManager = GameUIDefaults::GAME_LOOP->getAudioManager();

    audio_supported = true;
    if (!audio_supported) return;

    music_on = GetBoolPreference(kMusic,true);
    sound_on = GetBoolPreference(kSound,true);

    music_volume = ((float)GetIntPreference(kMusicVolume, 100))/100.0f;
    sound_volume = ((float)GetIntPreference(kSoundVolume, 100))/100.0f;

    if (music_on) loadMusic("pop.xm");

}

void AudioManager::close()
{
#ifdef USE_AUDIO
    if (!audio_supported) return;
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
        audio_manager::Sound *removed = chunkCache.removeOne();
        if (removed)
            delete removed;
    }
#endif
}

void AudioManager::clearMusicCache()
{
#ifdef USE_AUDIO
    if (!audio_supported) return;

    while (!musicCache.empty())
    {
        audio_manager::Music *removed = musicCache.removeOne();
        if (removed)
            delete removed;
    }
#endif
}

#ifdef UNUSED
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
#endif

void AudioManager::preloadMusic(const char *fileName)
{
    //if (!audio_supported) return;
    if (musicCache.contains(fileName)) return;

    String filePath = theCommander->getDataPathManager().getPath(FilePath("music").combine(fileName));
    audio_manager::Music *music   = m_audioManager->loadMusic(filePath);
    audio_manager::Music *removed = musicCache.add(fileName, music);
    if (removed)
        delete removed;
}

void AudioManager::loadMusic(const char *fileName)
{
  if ((!music_starting) && ((!audio_supported) || (music_current == fileName))) return;
    music_current = fileName;
    if (music_on)
    {
      preloadMusic(fileName);
      audio_manager::Music *music = musicCache.veryGet(fileName);
      if (music != NULL)
          m_audioManager->playMusic(music);
    }
}

void AudioManager::music(const char *command)
{
  static const int MSTART[6] = {1, 12, 15, 22, 34, 0x28};
  static const int MGAME[4] = { MSTART[4], MSTART[3], MSTART[0], MSTART[2] };
  static const int FGAME[4] = { MSTART[5], MSTART[3], MSTART[0], MSTART[2] };

  if (((!music_on) && (!audio_supported)) || (music_command == command)) return;
  music_command = command;
  if (music_command == "menu") {
      m_audioManager->setMusicPosition(MSTART[1]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "credits") {
      m_audioManager->setMusicPosition(MSTART[0]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "network menu") {
      m_audioManager->setMusicPosition(MSTART[2]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "level1") {
      m_audioManager->setMusicPosition(MGAME[0]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "level1-speed") {
      m_audioManager->setMusicPosition(FGAME[0]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "level2") {
      m_audioManager->setMusicPosition(MGAME[1]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "level2-speed") {
      m_audioManager->setMusicPosition(FGAME[1]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "level3") {
      m_audioManager->setMusicPosition(MGAME[2]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "level4") {
      m_audioManager->setMusicPosition(MGAME[3]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "level5") {
      m_audioManager->setMusicPosition(MGAME[0]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "level6") {
      m_audioManager->setMusicPosition(MGAME[1]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "level7") {
      m_audioManager->setMusicPosition(MGAME[2]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "level8") {
      m_audioManager->setMusicPosition(MGAME[3]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "level9") {
      m_audioManager->setMusicPosition(MGAME[0]);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  /*else if (music_command == "game") { keep music of interlevel for now.
      Mix_SetMusicPosition(MSTART[rand()%4]);
      m_audioManager->setMusicVolume ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }*/
  else if (music_command == "game won") {
      m_audioManager->setMusicPosition(15);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
  else if (music_command == "game over") {
      m_audioManager->setMusicPosition(15);
      m_audioManager->setMusicVolume(music_volume * 0.3);
  }
}

void AudioManager::preloadSound(const char *fileName, float volume)
{
    // if (!audio_supported) return;

    if (chunkCache.contains(fileName)) return;

    String filePath = theCommander->getDataPathManager().getPath(FilePath("sfx").combine(fileName));
    audio_manager::Sound *chunk   = m_audioManager->loadSound(filePath);
    //, (int)(volume * MIX_MAX_VOLUME));
    audio_manager::Sound *removed = chunkCache.add(fileName, chunk);
    if (removed)
        delete removed;
}

void AudioManager::playSound(const char *fileName, float volume, float balance)
{
    //if ((!audio_supported) || (!sound_on)) return;

    preloadSound(fileName, volume);
    std::string c = fileName;
    if (ios_fc::getTimeMs() - chunkCache.veryGetLastUse(c) < TIMEMS_BETWEEN_SAME_SOUND)
        return;

    audio_manager::Sound *chunk = chunkCache.veryGet(c);
    if (chunk != NULL) {
        m_audioManager->playSound(chunk, volume, balance);
    }
}

void AudioManager::musicVolume(float volume)
{
    //if (!audio_supported) return;
    if (music_on)
    {
        m_audioManager->setMusicVolume(volume);
    }
    music_volume = volume;
}

void AudioManager::soundVolume(float volume)
{
    //if (!audio_supported) return;
    if (sound_on)
    {
        m_audioManager->setSoundVolume(volume);
    }
    sound_volume = volume;
}

const char * AudioManager::musicVolumeKey(void) { return kMusicVolume; }
const char * AudioManager::soundVolumeKey(void) { return kSoundVolume; }
const char * AudioManager::musicOnOffKey(void)  { return kMusic; }
const char * AudioManager::soundOnOffKey(void)  { return kSound; }

void AudioManager::musicOnOff(bool state)
{
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
      m_audioManager->stopMusic();
  }
}

void AudioManager::soundOnOff(bool state)
{
  if ((!audio_supported) || (sound_on == state)) return;

  sound_on = state;
}

bool AudioManager::isMusicOn()
{
  return music_on;
}

bool AudioManager::isSoundOn()
{
  return sound_on;
}
