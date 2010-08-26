#ifndef _SDL_AUDIOMANAGER_H_
#define _SDL_AUDIOMANAGER_H_

#include "config.h"
#include "audiomanager.h"

#ifdef HAVE_SDL_SDL_MIXER_H
#include <SDL/SDL_mixer.h>
#else
#ifdef HAVE_SDL_MIXER_SDL_MIXER_H
#include <SDL_mixer/SDL_mixer.h>
#endif
#endif

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
private:
    bool   m_audio_supported;
    int    m_audio_rate;
    Uint16 m_audio_format;
    int    m_audio_channels;

    bool m_music_on;
    bool m_sound_on;
};

#endif // _SDL_AUDIOMANAGER_H_

