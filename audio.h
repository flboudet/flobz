#ifndef _AUDIO_H
#define _AUDIO_H

/// AUDIO

class AudioManager
{
  public:
    static void init(const char *dataFolder);
    static void close();
    
    static void preloadMusic(const char *fileName);
    static void playMusic(const char *fileName);
    static void clearMusicCache();

    static void preloadSound(const char *sName, float volume);
    static void playSound(const char *sName, float volume = 1.0, float balance = 0.0f);
    static void clearSoundCache();

    // volume in [0,1) interval
    static void musicVolume(float volume);
    static void soundVolume(float volume);

    static void musicOnOff(bool state) {} // TODO
    static void soundOnOff(bool state) {} // TODO
};

/*
class SoundPool
{
  public:
    SoundPool(unsigned poolSize = 256);
    ~SoundPool() { clearCache(); }
    
    // Purge the cache
    void clearCache();

    // Preload sound in the pool
    void preloadSound(const char *sName);

    // Play sound in file sName with specified volume and balance
    // 
    // left:   balance = -1
    // center: balance = 0
    // right:  balance = 1
    //
    // volume max: 1.0
    void playSound(const char *sName, float volume = 1.0, float balance = 0.0f);

  private:
    Sound *pool[];
};
*/

/*
/// les differents sons utilisables
extern Sound *sound_pop;
extern Sound *sound_bam1;
extern Sound *sound_fff;
extern Sound *sound_slide;
#define NB_YAHOOHOO1 4
#define NB_YAHOOHOO2 2
#define NB_YAHOOHOO3 1
extern Sound *sound_yahoohoo1[NB_YAHOOHOO1];
extern Sound *sound_yahoohoo2[NB_YAHOOHOO2];
extern Sound *sound_yahoohoo3[NB_YAHOOHOO3];
extern Sound *sound_splash[8];
extern Sound  *sound_bim[2];

void    audio_init ();
void    audio_music_start (int num);
void    audio_sound_play (Sound * s);
void    audio_close ();

/// volume compris entre 0 et 100;
void    audio_music_set_volume (int vol);

/// volume compris entre 0 et 100;
void    audio_set_volume (int vol);

void    audio_set_music_on_off (int on);
void    audio_set_sound_on_off (int on);


#define NB_MUSIC_THEME 2
void    audio_music_switch_theme(int theme_number);
*/

#endif
