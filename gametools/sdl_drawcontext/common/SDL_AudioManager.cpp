#include <stdlib.h>
#include "SDL_AudioManager.h"
#include "ios_exception.h"
#include "GTLog.h"
#include <iostream>

// Workaround for a sdl_mixer bug
// http://bugzilla.libsdl.org/show_bug.cgi?id=1499
//#define SDLMIKMOD_WORKAROUND
#ifdef SDLMIKMOD_WORKAROUND
#include <mikmod.h>
#endif

using namespace audio_manager;

SDL_AM_Music::SDL_AM_Music(Mix_Music *music)
    : m_music(music)
{
}

SDL_AM_Music::~SDL_AM_Music()
{
    Mix_FreeMusic(m_music);
}

SDL_AM_Sound::SDL_AM_Sound(Mix_Chunk *chunk)
    : m_chunk(chunk)
{
}

SDL_AM_Sound::~SDL_AM_Sound()
{
    Mix_FreeChunk(m_chunk);
}

SDL_AudioManager::SDL_AudioManager()
    : m_music_on(true), m_sound_on(true),
      m_musicVolume(1.), m_soundVolume(1.)
{
    m_audio_supported = (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) == 0);
    if (!m_audio_supported)
        return;
    //Mix_QuerySpec(&m_audio_rate, &m_audio_format, &m_audio_channels);
    Mix_AllocateChannels(16);

}

SDL_AudioManager::~SDL_AudioManager()
{
    Mix_CloseAudio();
}

void SDL_AudioManager::setMusicEnabled(bool enabled)
{
    if (! enabled)
        Mix_HaltMusic();
    m_music_on = enabled;
}

void SDL_AudioManager::setMusicVolume(float volume)
{
    m_musicVolume = volume;
    Mix_VolumeMusic((int)((float)MIX_MAX_VOLUME * volume));
}

bool SDL_AudioManager::getMusicEnabled() const
{
    return m_music_on;
}

float SDL_AudioManager::getMusicVolume() const
{
    return m_musicVolume;
}

Music *SDL_AudioManager::loadMusic(const char *fileName)
{
    GTLogTrace("++ loadMusic: %s\n%s", fileName, ios_fc::get_stack_trace().c_str());
    Mix_Music *music = Mix_LoadMUS(fileName);
    Music *result = new SDL_AM_Music(music);
    return result;
}

void SDL_AudioManager::playMusic(Music *music)
{
    if (!m_music_on)
        return;
    Mix_HaltMusic();
    if (music != NULL) {
        Mix_PlayMusic(static_cast<SDL_AM_Music *>(music)->m_music, -1);
#ifdef DISABLED
#ifdef SDLMIKMOD_WORKAROUND
        MODULE *mod;
        mod = Player_GetModule();
        if (mod != NULL)
            mod->loop = 1;
#endif
#endif
    }
}

void SDL_AudioManager::stopMusic()
{
    Mix_HaltMusic();
}

void SDL_AudioManager::setMusicPosition(double position)
{
    //Mix_SetMusicPosition(position);
}

void SDL_AudioManager::setSoundEnabled(bool enabled)
{
    m_sound_on = enabled;
}

void SDL_AudioManager::setSoundVolume(float volume)
{
}

bool SDL_AudioManager::getSoundEnabled() const
{
    return m_sound_on;
}

float SDL_AudioManager::getSoundVolume() const
{
    return m_soundVolume;
}

Sound *SDL_AudioManager::loadSound(const char *fileName)
{
    Mix_Chunk *chunk = Mix_LoadWAV(fileName);
    Sound *result = new SDL_AM_Sound(chunk);
    return result;
}

void SDL_AudioManager::playSound(Sound *sound, float volume, float balance)
{
    if (! m_sound_on)
        return;
    int channel = Mix_PlayChannel (-1, static_cast<SDL_AM_Sound *>(sound)->m_chunk, 0);
    Uint8 leftVolume = (Uint8)(255.*((-balance + 1.)/2.));
    Mix_SetPanning(channel, leftVolume * volume, (255-leftVolume) * volume);
}
