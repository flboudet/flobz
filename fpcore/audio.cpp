#include <string.h>
#include "audio.h"
#include <vector>
#include <map>
#include <string>
#include "FPCommander.h"

#define TIMEMS_BETWEEN_SAME_SOUND 10.0

static std::map<std::string, double> lastUsedTimestamp;
static bool   audio_supported = false;

static float sound_volume   = 1.0f;
static float music_volume   = 1.0f;

static bool sound_on = false;
static bool music_on = false;
static bool music_starting = false;

static std::string music_current = "";
static std::string music_command = "";

static const char * kMusicVolume = "AudioManager.Music.Volume";
static const char * kSoundVolume = "AudioManager.FX.Volume";
static const char * kMusic       = "AudioManager.Music.State";
static const char * kSound       = "AudioManager.FX.State";

audio_manager::AudioManager * AudioManager::m_audioManager;

void AudioManager::init()
{
    m_audioManager = GameUIDefaults::GAME_LOOP->getAudioManager();
    audio_supported = true;
    if (!audio_supported) return;
    music_on = theCommander->getPreferencesManager()->getBoolPreference(kMusic,true);
    sound_on = theCommander->getPreferencesManager()->getBoolPreference(kSound,true);
    music_volume = ((float)(theCommander->getPreferencesManager()->getIntPreference(kMusicVolume, 100)))/100.0f;
    sound_volume = ((float)(theCommander->getPreferencesManager()->getIntPreference(kSoundVolume, 100)))/100.0f;
    if (music_on)
        loadMusic("pop.xm");
}

void AudioManager::close()
{
    if (!audio_supported) return;
}

AudioManager::AudioManager()
{
    GlobalNotificationCenter.addListener(String(kMusicVolume), this);
    GlobalNotificationCenter.addListener(String(kSoundVolume), this);
    GlobalNotificationCenter.addListener(String(kMusic), this);
    GlobalNotificationCenter.addListener(String(kSound), this);
}

AudioManager::~AudioManager()
{
    GlobalNotificationCenter.removeListener(String(kMusicVolume), this);
    GlobalNotificationCenter.removeListener(String(kSoundVolume), this);
    GlobalNotificationCenter.removeListener(String(kMusic), this);
    GlobalNotificationCenter.removeListener(String(kSound), this);
}

void AudioManager::notificationOccured(String identifier, void * context)
{
    if (identifier == kMusicVolume) {
        m_audioManager->setMusicVolume((float)*(int *)context);
    }
    else if (identifier == kSoundVolume) {
        m_audioManager->setSoundVolume((float)*(int *)context);
    }
    else if (identifier == kMusic) {
        bool enabled = *(bool *)context;
        m_audioManager->setMusicEnabled(enabled);
        if (enabled) {
            loadMusic("pop.xm");
            std::string prevCommand = music_command;
            music_command = "";
            music(music_command.c_str());
        }
    }
    else if (identifier == kSound) {
        m_audioManager->setSoundEnabled(*(bool *)context);
    }
}

void AudioManager::preloadMusic(const char *fileName)
{
    theCommander->cacheMusic(FilePath("music").combine(fileName));
}

void AudioManager::loadMusic(const char *fileName)
{
    MusicRef music = theCommander->getMusic(FilePath("music").combine(fileName));
    if (music.get() != NULL)
        m_audioManager->playMusic(music);
}

void AudioManager::music(const char *command)
{
  static const int MSTART[6] = {1, 12, 15, 22, 34, 0x28};
  static const int MGAME[4] = { MSTART[4], MSTART[3], MSTART[0], MSTART[2] };
  static const int FGAME[4] = { MSTART[5], MSTART[3], MSTART[0], MSTART[2] };

  if ((!audio_supported) || (music_command == command)) return;
  music_command = command;
  if (music_command == "menu") {
      m_audioManager->setMusicPosition(MSTART[1]);
  }
  else if (music_command == "credits") {
      m_audioManager->setMusicPosition(MSTART[0]);
  }
  else if (music_command == "network menu") {
      m_audioManager->setMusicPosition(MSTART[2]);
  }
  else if (music_command == "level1") {
      m_audioManager->setMusicPosition(MGAME[1]);
  }
  //else if (music_command == "level1-speed") {
  //    m_audioManager->setMusicPosition(FGAME[1]);
  //}
  else if (music_command == "level2") {
      m_audioManager->setMusicPosition(MGAME[0]);
  }
  else if (music_command == "level2-speed") {
      m_audioManager->setMusicPosition(FGAME[0]);
  }
  else if (music_command == "level3") {
      m_audioManager->setMusicPosition(MGAME[2]);
  }
  else if (music_command == "level4") {
      m_audioManager->setMusicPosition(MGAME[3]);
  }
  else if (music_command == "level5") {
      m_audioManager->setMusicPosition(MGAME[1]);
  }
  else if (music_command == "level6") {
      m_audioManager->setMusicPosition(MGAME[0]);
  }
  else if (music_command == "level6-speed") {
      m_audioManager->setMusicPosition(FGAME[0]);
  }
  else if (music_command == "level7") {
      m_audioManager->setMusicPosition(MGAME[2]);
  }
  else if (music_command == "level8") {
      m_audioManager->setMusicPosition(MGAME[0]);
  }
  else if (music_command == "level8-speed") {
      m_audioManager->setMusicPosition(FGAME[0]);
  }
  else if (music_command == "level9") {
      m_audioManager->setMusicPosition(MGAME[1]);
  }
  /*else if (music_command == "game") { keep music of interlevel for now.
      Mix_SetMusicPosition(MSTART[rand()%4]);
      m_audioManager->setMusicVolume ((int) ((float)MIX_MAX_VOLUME * music_volume * 0.7));
  }*/
  else if (music_command == "game won") {
      m_audioManager->setMusicPosition(15);
  }
  else if (music_command == "game over") {
      m_audioManager->setMusicPosition(15);
  }
}

void AudioManager::preloadSound(const char *fileName, float volume)
{
    theCommander->cacheSound(FilePath("sfx").combine(fileName));
}

void AudioManager::playSound(const char *fileName, float volume, float balance)
{
    double currentTime = ios_fc::getTimeMs();
    std::map<std::string, double>::iterator iter =
        lastUsedTimestamp.find(fileName);
    if ((iter != lastUsedTimestamp.end())
        && (currentTime - iter->second < TIMEMS_BETWEEN_SAME_SOUND))
        return;
    SoundRef sound   = theCommander->getSound(FilePath("sfx").combine(fileName));
    m_audioManager->playSound(sound, volume, balance);
    lastUsedTimestamp[fileName] = currentTime;
}



const char * AudioManager::musicVolumeKey(void) { return kMusicVolume; }
const char * AudioManager::soundVolumeKey(void) { return kSoundVolume; }
const char * AudioManager::musicOnOffKey(void)  { return kMusic; }
const char * AudioManager::soundOnOffKey(void)  { return kSound; }


