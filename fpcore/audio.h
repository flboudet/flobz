#ifndef _AUDIO_H
#define _AUDIO_H

#include <map>
#include "audiomanager.h"
#include "NotifyCenter.h"
#include "FPResources.h"

/**
 * Jukebox is a dictionary of tracks
 * with the methods to play them
 */
class Jukebox
{
public:
    void registerTrack(const char *fileName, const char *trackName, int position = 0);
    void playTrack(const char *trackName);
    void playTrack();
private:
    struct JukeboxTrack {
        JukeboxTrack(const char *fileName, int position)
            : fileName(fileName), position(position) {}
        JukeboxTrack() {}
        std::string fileName;
        int position;
    };
    std::map<std::string, JukeboxTrack> m_regTracks;
    MusicRef m_playingMusic;
    std::string m_playingMusicName;
};

/**
 * AudioHelper manages sound and music settings
 * It also provides a handy playSound() method
 */
class AudioHelper : public gameui::NotificationResponder
{
public:
    static const char * musicVolumeKey(void);
    static const char * soundVolumeKey(void);
    static const char * musicOnOffKey(void);
    static const char * soundOnOffKey(void);

public:
    AudioHelper();
    ~AudioHelper();
    // Helper functions
    void playSound(const char *sName, float volume = 1.0, float balance = 0.0f);
    // NotificationResponder implementation
    void notificationOccured(String identifier, void * context);

private:
    audio_manager::AudioManager *m_audioManager;
    float m_sound_volume;
    float m_music_volume;
    bool m_sound_on;
    bool m_music_on;
    std::map<std::string, double> m_lastUsedTimestamp;
};


#endif
