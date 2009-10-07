#ifndef _AUDIOMANAGER_H
#define _AUDIOMANAGER_H

namespace audio_manager
{

class Music
{
public:
    virtual ~Music() {}
};

class Sound
{
public:
    virtual ~Sound() {}
};

class AudioManager
{
public:
    virtual ~AudioManager() {}
    virtual void setMusicEnabled(bool enabled) = 0;
    virtual void setMusicVolume(float volume) = 0;
    virtual Music *loadMusic(const char *fileName) = 0;
    virtual void playMusic(Music *music) = 0;
    virtual void stopMusic() = 0;
    virtual void setMusicPosition(double position) = 0;

    virtual void setSoundEnabled(bool enabled) = 0;
    virtual void setSoundVolume(float volume) = 0;
    virtual Sound *loadSound(const char *fileName) = 0;
    virtual void playSound(Sound *sound, float volume, float balance) = 0;
};

}

#endif // _AUDIOMANAGER_H

