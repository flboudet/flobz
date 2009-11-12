#ifndef _OpenGL_AUDIOMANAGER_H_
#define _OpenGL_AUDIOMANAGER_H_

#include "audiomanager.h"

class OpenGL_AM_Music : public audio_manager::Music
{
public:
    OpenGL_AM_Music();
    virtual ~OpenGL_AM_Music();
};

class OpenGL_AM_Sound : public audio_manager::Sound
{
public:
    OpenGL_AM_Sound();
    virtual ~OpenGL_AM_Sound();
};

class OpenGL_AudioManager : public audio_manager::AudioManager
{
public:
    OpenGL_AudioManager();
    virtual ~OpenGL_AudioManager();
    virtual void setMusicEnabled(bool enabled);
    virtual void setMusicVolume(float volume);
    virtual audio_manager::Music *loadMusic(const char *fileName);
    virtual void playMusic(audio_manager::Music *music);
    virtual void stopMusic();
    virtual void setMusicPosition(double position);

    virtual void setSoundEnabled(bool enabled);
    virtual void setSoundVolume(float volume);
    virtual audio_manager::Sound *loadSound(const char *fileName);
    virtual void playSound(audio_manager::Sound *sound, float volume = 1.0, float balance = 0.0f);
};

#endif // _OpenGL_AUDIOMANAGER_H_


