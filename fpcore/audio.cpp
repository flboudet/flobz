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
    m_regTracks[trackName] = JukeboxTrack(fileName, position);
}

void Jukebox::playTrack(const std::string &name)
{
    if (m_playingMusicName == name)
        return;
    GTLogTrace("Playing track %s", name);
    MusicRef music;
    int pos = 0;
    std::map<std::string, JukeboxTrack>::const_iterator iter = m_regTracks.find(name);
    if (iter != m_regTracks.end()) {
        music = theCommander->getMusic(iter->second.fileName.c_str());
        pos = iter->second.position;
    }
    else
        music = theCommander->getMusic(name);
    if (music.get() != m_playingMusic.get()) {
        GameUIDefaults::GAME_LOOP->getAudioManager()->playMusic(music);
        m_playingMusic = music;
    }
    GameUIDefaults::GAME_LOOP->getAudioManager()->setMusicPosition(pos);
    m_playingMusicName = name;
}

void Jukebox::playTrack()
{
    playTrack("");
    playTrack(m_playingMusicName.c_str());
}

AudioHelper::AudioHelper()
{
    // Listen to notifications
    GlobalNotificationCenter.addListener((kMusicVolume), this);
    GlobalNotificationCenter.addListener((kSoundVolume), this);
    GlobalNotificationCenter.addListener((kMusic), this);
    GlobalNotificationCenter.addListener((kSound), this);
    // Initialize preferences
    m_audioManager = GameUIDefaults::GAME_LOOP->getAudioManager();
    m_music_on = theCommander->getPreferencesManager()->getBoolPreference(kMusic,true);
    m_sound_on = theCommander->getPreferencesManager()->getBoolPreference(kSound,true);
    m_music_volume = ((float)(theCommander->getPreferencesManager()->getIntPreference(kMusicVolume, 100)))/100.0f;
    m_sound_volume = ((float)(theCommander->getPreferencesManager()->getIntPreference(kSoundVolume, 100)))/100.0f;
    m_audioManager->setSoundEnabled(m_sound_on);
    m_audioManager->setMusicEnabled(m_music_on);
    m_audioManager->setSoundVolume(m_sound_volume);
    m_audioManager->setMusicVolume(m_music_volume);
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
        m_lastUsedTimestamp.find(fileName);
    if ((iter != m_lastUsedTimestamp.end())
        && (currentTime - iter->second < TIMEMS_BETWEEN_SAME_SOUND))
        return;
    SoundRef sound   = theCommander->getSound(FilePath("sfx").combine(fileName));
    m_audioManager->playSound(sound, volume, balance);
    m_lastUsedTimestamp[fileName] = currentTime;
}

void AudioHelper::notificationOccured(String identifier, void * context)
{
    if (identifier == kMusicVolume) {
        m_audioManager->setMusicVolume((float)*(int *)context);
    }
    else if (identifier == kSoundVolume) {
        m_audioManager->setSoundVolume((float)*(int *)context);
    }
    else if (identifier == kMusic) {
        bool enabled = *(bool *)context;
        m_audioManager->setMusicEnabled(enabled);
        if (enabled) {
            theCommander->playMusicTrack();
        }
    }
    else if (identifier == kSound) {
        m_audioManager->setSoundEnabled(*(bool *)context);
    }
}



const char * AudioHelper::musicVolumeKey(void) { return kMusicVolume; }
const char * AudioHelper::soundVolumeKey(void) { return kSoundVolume; }
const char * AudioHelper::musicOnOffKey(void)  { return kMusic; }
const char * AudioHelper::soundOnOffKey(void)  { return kSound; }


