#ifndef _AUDIO_H
#define _AUDIO_H

#include "audiomanager.h"
#include "NotifyCenter.h"

/// AUDIO

class AudioManager : public gameui::NotificationResponder
{
public:
    AudioManager();
    ~AudioManager();
    void notificationOccured(String identifier, void * context);

  static void init();
  static void close();

  static void preloadMusic(const char *fileName);
  static void loadMusic(const char *fileName);
  static void music(const char *command);

  static void preloadSound(const char *sName, float volume);
  static void playSound(const char *sName, float volume = 1.0, float balance = 0.0f);

  static const char * musicVolumeKey(void);
  static const char * soundVolumeKey(void);
  static const char * musicOnOffKey(void);
  static const char * soundOnOffKey(void);

private:
    static audio_manager::AudioManager *m_audioManager;
};


#endif
