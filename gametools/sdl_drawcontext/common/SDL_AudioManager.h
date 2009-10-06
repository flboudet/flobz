#ifndef _SDL_AUDIOMANAGER_H_
#define _SDL_AUDIOMANAGER_H_

#include "audiomanager.h"
#include <SDL_mixer.h>

class SDL_AM_Music : public audio_manager::Music
{
public:
    SDL_AM_Music(Mix_Music *music);
    virtual ~SDL_AM_Music();
    Mix_Music *m_music;
};

class SDL_AM_Sound : public audio_manager::Sound
{
public:
    SDL_AM_Sound(Mix_Chunk *chunk);
    virtual ~SDL_AM_Sound();
    Mix_Chunk *m_chunk;
};

class SDL_AudioManager : public audio_manager::AudioManager
{
public:
    SDL_AudioManager();
    virtual ~SDL_AudioManager();
    virtual void setMusicEnabled(bool enabled) = 0;
    virtual void setMusicVolume(float volume) = 0;
    virtual audio_manager::Music *loadMusic(const char *fileName) = 0;
    virtual void playMusic(audio_manager::Music *music) = 0;
    virtual void setMusicPosition(double position) = 0;

    virtual void setSoundEnabled(bool enabled) = 0;
    virtual void setSoundVolume(float volume) = 0;
    virtual audio_manager::Sound *loadSound(const char *fileName) = 0;
    virtual void playSound(audio_manager::Sound *sound, float volume, float balance) = 0;
private:
    bool   m_audio_supported;
    int    m_audio_rate;
    Uint16 m_audio_format;
    int    m_audio_channels;

    bool m_music_on;
};

#endif // _SDL_AUDIOMANAGER_H_

