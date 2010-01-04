#ifndef _PUYOCOMMANDER
#define _PUYOCOMMANDER

#include "gameui.h"
#include "Frame.h"
#include "DataPathManager.h"
#include "LocalizedDictionary.h"
#include "GameCursor.h"
#include "ScreenTransition.h"
#include "audio.h"
#include "ResourceManager.h"
#include <memory>

using namespace gameui;

/**
 * Resource key for IosSurface management.
 * Makes an unique key for an ImageType, path, Ability association
 */
class IosSurfaceResourceKey {
public:
    IosSurfaceResourceKey(ImageType type, const std::string &path, ImageSpecialAbility specialAbility)
        : path(path), type(type), specialAbility(specialAbility) {}
    bool operator =(const IosSurfaceResourceKey k) const {
        return  ((this->path == k.path) &&
                 (this->type == k.type) &&
                 (this->specialAbility == k.specialAbility));
    }
    bool operator < (const IosSurfaceResourceKey k) const {
        if (this->path == k.path) {
            if (this->type == k.type)
                return this->specialAbility < k.specialAbility;
            return this->type < k.type;
        }
        return (this->path < k.path);
    }
    std::string path;
    ImageType type;
    ImageSpecialAbility specialAbility;
};

/**
 * Factory for IosSurface resources
 */
class IosSurfaceFactory : public ResourceFactory<IosSurface, IosSurfaceResourceKey>
{
public:
    IosSurfaceFactory(DataPathManager &dataPathManager)
        : m_dataPathManager(dataPathManager) {}
    virtual IosSurface *create(const IosSurfaceResourceKey &resourceKey);
    virtual void destroy(IosSurface *res);
private:
    DataPathManager &m_dataPathManager;
};

/**
 * Factory for Sound resources
 */
class SoundFactory : public ResourceFactory<audio_manager::Sound>
{
public:
    SoundFactory(DataPathManager &dataPathManager)
        : m_dataPathManager(dataPathManager) {}
    virtual audio_manager::Sound *create(const std::string &path);
    virtual void destroy(audio_manager::Sound *res);
private:
    DataPathManager &m_dataPathManager;
};

/**
 * Factory for Music resources
 */
class MusicFactory : public ResourceFactory<audio_manager::Music>
{
public:
    MusicFactory(DataPathManager &dataPathManager)
        : m_dataPathManager(dataPathManager) {}
    virtual audio_manager::Music *create(const std::string &path);
    virtual void destroy(audio_manager::Music *res);
private:
    DataPathManager &m_dataPathManager;
};

// IosSurface resources
typedef ResourceReference<IosSurface> IosSurfaceRef;
typedef ResourceManager<IosSurface, IosSurfaceResourceKey> IosSurfaceResourceManager;
// Sound resources
typedef ResourceReference<audio_manager::Sound> SoundRef;
typedef ResourceManager<audio_manager::Sound> SoundResourceManager;
// Music resources
typedef ResourceReference<audio_manager::Music> MusicRef;
typedef ResourceManager<audio_manager::Music> MusicResourceManager;

class PuyoCommander
{
  public:
    PuyoCommander(String dataDir, int maxDataPackNumber=-1);
    void initWithGUI(bool fullscreen);
    void initWithoutGUI();

    virtual ~PuyoCommander();

    bool getMusic();
    bool getSoundFx();
    String getFullScreenKey(void) const;
    bool getGlSDL() const { return useGL; }
    void setGlSDL(bool useGL);

    // Cursor management
    void registerCursor(AbstractCursor *cursor);
    void setCursorVisible(bool visible);

    // Transition widget factory
    virtual ScreenTransitionWidget *createScreenTransition(Screen &fromScreen) const;

    // Resource managers
    void cacheSurface(ImageType type, const char *path, ImageSpecialAbility specialAbility = 0);
    IosSurfaceRef getSurface(ImageType type, const char *path, ImageSpecialAbility specialAbility = 0);
    void cacheSound(const char *path);
    SoundRef getSound(const char *path);
    void cacheMusic(const char *path);
    MusicRef getMusic(const char *path);

    // Data path management
    const DataPathManager &getDataPathManager() { return dataPathManager; }
    const char * getLocalizedString(const char * originalString) const;

    // Common resources accessor
    const FramePicture *getWindowFramePicture() const { return m_windowFramePicture.get(); }
    const FramePicture *getButtonFramePicture() const { return m_buttonIdleFramePicture.get(); }
    const FramePicture *getButtonDownFramePicture() const { return m_buttonDownFramePicture.get(); }
    const FramePicture *getButtonOverFramePicture() const { return m_buttonOverFramePicture.get(); }
    const FramePicture *getEditFieldFramePicture() const { return m_textFieldIdleFramePicture.get(); }
    const FramePicture *getEditFieldOverFramePicture() const { return m_textFieldIdleFramePicture.get(); }
    const FramePicture *getSeparatorFramePicture() const { return m_separatorFramePicture.get(); }
    const FramePicture *getListFramePicture() const { return m_listFramePicture.get(); }
    IosSurface * getSwitchOnPicture() { return m_switchOnImage; }
    IosSurface * getSwitchOffPicture() { return m_switchOffImage; }
    IosSurface * getRadioOnPicture() { return m_radioOnImage; }
    IosSurface * getRadioOffPicture() { return m_radioOffImage; }
    IosSurface * getUpArrow() { return m_upArrow; }
    IosSurface * getDownArrow() { return m_downArrow; }
    IosSurface * getLeftArrow() { return m_leftArrow; }
    IosSurface * getRightArrow() { return m_rightArrow; }
    audio_manager::Sound * getWhipSound() const { return m_whipSound; }
    audio_manager::Sound * getWhopSound() const { return m_whopSound; }
  protected:
    // Resource manager factory
    virtual void createResourceManagers();
    // Resource Managers
    IosSurfaceFactory m_surfaceFactory;
    std::auto_ptr<IosSurfaceResourceManager> m_surfaceResManager;
    SoundFactory m_soundFactory;
    std::auto_ptr<SoundResourceManager> m_soundResManager;
    MusicFactory m_musicFactory;
    std::auto_ptr<MusicResourceManager> m_musicResManager;    
  private:

    friend class SinglePlayerGameAction;
    friend class NetGameAction;
    friend class MainMenu;
    friend class NetworkGameMenu;

    void loadPreferences(bool fs);
    void initLocale();
    void initAudio();
    void initFonts();

    DataPathManager dataPathManager;

    MessageBox *mbox;
    GameLoop   *loop;
    LocalizedDictionary * locale;

    AudioManager globalAudioManager;

    bool useGL;
    IosSurfaceRef m_frameImage;
    IosSurfaceRef m_buttonIdleImage, m_buttonDownImage, m_buttonOverImage;
    IosSurfaceRef m_textFieldIdleImage;
    IosSurfaceRef m_separatorImage;
    IosSurfaceRef m_listIdleImage;
    IosSurfaceRef m_switchOnImage, m_switchOffImage;
    IosSurfaceRef m_radioOnImage, m_radioOffImage;
    IosSurfaceRef m_upArrow, m_downArrow, m_leftArrow, m_rightArrow;
    std::auto_ptr<FramePicture> m_windowFramePicture;
    std::auto_ptr<FramePicture> m_buttonIdleFramePicture;
    std::auto_ptr<FramePicture> m_buttonDownFramePicture;
    std::auto_ptr<FramePicture> m_buttonOverFramePicture;
    std::auto_ptr<FramePicture> m_textFieldIdleFramePicture;
    std::auto_ptr<FramePicture> m_separatorFramePicture;
    std::auto_ptr<FramePicture> m_listFramePicture;

    SoundRef m_slideSound;
    SoundRef m_whipSound;
    SoundRef m_whopSound;

    AbstractCursor *m_cursor;
};

extern class PuyoCommander *theCommander;

#endif // _PUYOCOMMANDER
