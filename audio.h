#ifndef _AUDIO_H
#define _AUDIO_H

/// AUDIO

class AudioManager
{
  public:

  static void init();
  static void close();
  
  static void preloadMusic(const char *fileName);
  static void playMusic(const char *fileName);
  static void clearMusicCache();

  static void preloadSound(const char *sName, float volume);
  static void playSound(const char *sName, float volume = 1.0, float balance = 0.0f);
  static void clearSoundCache();

  // volume in [0,1] interval
  static void musicVolume(float volume);
  static void soundVolume(float volume);

  static void musicOnOff(bool state);
  static void soundOnOff(bool state);

  static bool isMusicOn();
  static bool isSoundOn();
};


#endif
