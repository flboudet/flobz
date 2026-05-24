#include <string.h>
#include "audio.h"
#include <vector>
#include <string>
#include "FPCommander.h"
#include "GTLog.h"

#define TIMEMS_BETWEEN_SAME_SOUND 100.0
static const char * kMusicVolume = "AudioManager.Music.Volume";
static const char * kSoundVolume = "AudioManager.FX.Volume";
static const char * kMusic       = "AudioManager.Music.State";
static const char * kSound       = "AudioManager.FX.State";

void Jukebox::registerTrack(const std::string &trackName, const std::string &fileName, int position)
{
    GTLogTrace("Registering track %s, file %s, position %d", trackName, fileName, position);
    _regTracks[trackName] = JukeboxTrack(fileName, position);
}

void Jukebox::playTrack(const std::string &name)
{
    if (_playingMusicName == name)
        return;
    GTLogTrace("Playing track %s", name);
    MusicRef music;
    int pos = 0;
    std::map<std::string, JukeboxTrack>::const_iterator iter = _regTracks.find(name);
    if (iter != _regTracks.end()) {
        music = theCommander->getMusic(iter->second.fileName.c_str());
        pos = iter->second.position;
    }
    else
        music = theCommander->getMusic(name);
    if (music.get() != _playingMusic.get()) {
        GameUIDefaults::GAME_LOOP->getAudioManager()->playMusic(music);
        _playingMusic = music;
    }
    GameUIDefaults::GAME_LOOP->getAudioManager()->setMusicPosition(pos);
    _playingMusicName = name;
}

void Jukebox::playTrack()
{
    playTrack("");
    playTrack(_playingMusicName.c_str());
}

AudioHelper::AudioHelper()
{
    // Listen to notifications
    GlobalNotificationCenter.addListener((kMusicVolume), this);
    GlobalNotificationCenter.addListener((kSoundVolume), this);
    GlobalNotificationCenter.addListener((kMusic), this);
    GlobalNotificationCenter.addListener((kSound), this);
    // Initialize preferences
    _audioManager = GameUIDefaults::GAME_LOOP->getAudioManager();
    _music_on = theCommander->getPreferencesManager()->getBoolPreference(kMusic,true);
    _sound_on = theCommander->getPreferencesManager()->getBoolPreference(kSound,true);
    _music_volume = ((float)(theCommander->getPreferencesManager()->getIntPreference(kMusicVolume, 100)))/100.0f;
    _sound_volume = ((float)(theCommander->getPreferencesManager()->getIntPreference(kSoundVolume, 100)))/100.0f;
    _audioManager->setSoundEnabled(_sound_on);
    _audioManager->setMusicEnabled(_music_on);
    _audioManager->setSoundVolume(_sound_volume);
    _audioManager->setMusicVolume(_music_volume);
}

AudioHelper::~AudioHelper()
{
    GlobalNotificationCenter.removeListener((kMusicVolume), this);
    GlobalNotificationCenter.removeListener((kSoundVolume), this);
    GlobalNotificationCenter.removeListener((kMusic), this);
    GlobalNotificationCenter.removeListener((kSound), this);
}

void AudioHelper::playSound(const std::string &fileName, float volume, float balance)
{
    double currentTime = ios_fc::getTimeMs();
    std::map<std::string, double>::iterator iter =
        _lastUsedTimestamp.find(fileName);
    if ((iter != _lastUsedTimestamp.end())
        && (currentTime - iter->second < TIMEMS_BETWEEN_SAME_SOUND))
        return;
    SoundRef sound   = theCommander->getSound(FilePath("sfx").combine(fileName));
    _audioManager->playSound(sound, volume, balance);
    _lastUsedTimestamp[fileName] = currentTime;
}

void AudioHelper::notificationOccured(const std::string &identifier, void * context)
{
    if (identifier == kMusicVolume) {
        _audioManager->setMusicVolume((float)*(int *)context);
    }
    else if (identifier == kSoundVolume) {
        _audioManager->setSoundVolume((float)*(int *)context);
    }
    else if (identifier == kMusic) {
        bool enabled = *(bool *)context;
        _audioManager->setMusicEnabled(enabled);
        if (enabled) {
            theCommander->playMusicTrack();
        }
    }
    else if (identifier == kSound) {
        _audioManager->setSoundEnabled(*(bool *)context);
    }
}



const char * AudioHelper::musicVolumeKey(void) { return kMusicVolume; }
const char * AudioHelper::soundVolumeKey(void) { return kSoundVolume; }
const char * AudioHelper::musicOnOffKey(void)  { return kMusic; }
const char * AudioHelper::soundOnOffKey(void)  { return kSound; }


