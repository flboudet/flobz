#ifndef _NULLAUDIOMANAGER_H
#define _NULLAUDIOMANAGER_H

#include "audiomanager.h"

class NullAudioManager : public audio_manager::AudioManager
{
public:
    virtual void setMusicEnabled(bool enabled) {}
    virtual void setMusicVolume(float volume)  {}
    virtual audio_manager::Music *loadMusic(const char *fileName) {
        return new audio_manager::Music();
    }
    virtual void playMusic(audio_manager::Music *music) {}
    virtual void stopMusic() {}
    virtual void setMusicPosition(double position) {}

    virtual void setSoundEnabled(bool enabled) {}
    virtual void setSoundVolume(float volume)  {}
    virtual audio_manager::Sound *loadSound(const char *fileName) {
        return new audio_manager::Sound();
    }
    virtual void playSound(audio_manager::Sound *sound, float volume, float balance) {}
};

#endif // _NULLAUDIOMANAGER_H

