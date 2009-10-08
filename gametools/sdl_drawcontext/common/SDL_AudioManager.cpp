#include <stdlib.h>
#include "SDL_AudioManager.h"

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
    : m_music_on(true)
{
    m_audio_supported = (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) == 0);
    if (!m_audio_supported)
        return;
    Mix_QuerySpec(&m_audio_rate, &m_audio_format, &m_audio_channels);
    Mix_AllocateChannels(16);

}

SDL_AudioManager::~SDL_AudioManager()
{
    Mix_CloseAudio();
}

void SDL_AudioManager::setMusicEnabled(bool enabled)
{
}

void SDL_AudioManager::setMusicVolume(float volume)
{
    Mix_VolumeMusic((int)((float)MIX_MAX_VOLUME * volume));
}

Music *SDL_AudioManager::loadMusic(const char *fileName)
{
    Mix_Music *music = Mix_LoadMUS(fileName);
    Music *result = new SDL_AM_Music(music);
    return result;
}

void SDL_AudioManager::playMusic(Music *music)
{
    if (!m_music_on)
        return;
    Mix_HaltMusic();
    if (music != NULL)
        Mix_PlayMusic(static_cast<SDL_AM_Music *>(music)->m_music, -1);
}

void SDL_AudioManager::stopMusic()
{
    Mix_HaltMusic();
}

void SDL_AudioManager::setMusicPosition(double position)
{
    Mix_SetMusicPosition(position);
}

void SDL_AudioManager::setSoundEnabled(bool enabled)
{
}

void SDL_AudioManager::setSoundVolume(float volume)
{
}

Sound *SDL_AudioManager::loadSound(const char *fileName)
{
    Mix_Chunk *chunk = Mix_LoadWAV(fileName);
    Sound *result = new SDL_AM_Sound(chunk);
    return result;
}

void SDL_AudioManager::playSound(Sound *sound, float volume, float balance)
{
    int channel = Mix_PlayChannel (-1, static_cast<SDL_AM_Sound *>(sound)->m_chunk, 0);
    Uint8 leftVolume = (Uint8)(255.*((-balance + 1.)/2.));
    Mix_SetPanning(channel, leftVolume, 255-leftVolume);
}

