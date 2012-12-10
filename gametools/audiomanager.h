#ifndef _AUDIOMANAGER_H
#define _AUDIOMANAGER_H

namespace audio_manager
{

enum AudioPolicy
{
    UsesOutput = 0x00,
    UsesInputAndOutput = 0x01
};
enum AudioPolicyModifier
{
    ForceOutputOnSpeaker = 0x0100,
    MixWithOther = 0x1000
};
    
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
    virtual bool getMusicEnabled() const = 0;
    virtual float getMusicVolume() const = 0;
    virtual Music *loadMusic(const char *fileName) = 0;
    virtual void playMusic(Music *music) = 0;
    virtual void stopMusic() = 0;
    virtual void setMusicPosition(double position) = 0;

    virtual void setSoundEnabled(bool enabled) = 0;
    virtual void setSoundVolume(float volume) = 0;
    virtual bool getSoundEnabled() const = 0;
    virtual float getSoundVolume() const = 0;
    virtual Sound *loadSound(const char *fileName) = 0;
    virtual void playSound(Sound *sound, float volume = 1.0, float balance = 0.0f) = 0;
};

}

#endif // _AUDIOMANAGER_H

