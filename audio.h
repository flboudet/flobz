#ifndef _AUDIO_H
#define _AUDIO_H

#include "NotifyCenter.h"

/// AUDIO

class AudioManager : public gameui::NotificationResponder
{
  public:

  AudioManager();
  ~AudioManager();

  static void init();
  static void close();
  
  static void preloadMusic(const char *fileName);
  static void playMusic(const char *fileName);
  static void clearMusicCache();

  static void preloadSound(const char *sName, float volume);
  static void playSound(const char *sName, float volume = 1.0, float balance = 0.0f);
  static void clearSoundCache();

  static const char * musicVolumeKey(void);
  static const char * soundVolumeKey(void);
  static const char * musicOnOffKey(void);
  static const char * soundOnOffKey(void);
    
  static void musicVolume(float volume); // Volume in [0.0f, 1.0f] interval
  static void soundVolume(float volume); // Volume in [0.0f, 1.0f] interval
  static void musicOnOff(bool state);
  static void soundOnOff(bool state);
    
  static bool isMusicOn();
  static bool isSoundOn();

  void notificationOccured(String identifier, void * context);

};


#endif
