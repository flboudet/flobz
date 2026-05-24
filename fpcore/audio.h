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
    void registerTrack(const std::string &fileName, const std::string &trackName, int position = 0);
    void playTrack(const std::string &trackName);
    void playTrack();
private:
    struct JukeboxTrack {
        JukeboxTrack(const std::string &fileName, int position)
            : fileName(fileName), position(position) {}
        JukeboxTrack() {}
        std::string fileName;
        int position;
    };
    std::map<std::string, JukeboxTrack> _regTracks;
    MusicRef _playingMusic;
    std::string _playingMusicName;
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
    void playSound(const std::string &sName, float volume = 1.0, float balance = 0.0f);
    // NotificationResponder implementation
    void notificationOccured(const std::string &identifier, void * context);

private:
    audio_manager::AudioManager *_audioManager;
    float _sound_volume;
    float _music_volume;
    bool _sound_on;
    bool _music_on;
    std::map<std::string, double> _lastUsedTimestamp;
};


#endif
