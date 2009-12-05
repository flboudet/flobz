#ifndef _PUYOCOMMANDER
#define _PUYOCOMMANDER

#include "gameui.h"
#include "Frame.h"
#include "PuyoStory.h"
#include "DataPathManager.h"
#include "LocalizedDictionary.h"
#include "GameCursor.h"
#include "audio.h"
#include "MainScreen.h"
#include <memory>

using namespace gameui;

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
    IosSurface   *m_frameImage, *m_buttonIdleImage, *m_buttonDownImage, *m_buttonOverImage;
    IosSurface   *m_textFieldIdleImage;
    IosSurface   *m_separatorImage;
    IosSurface   *m_listIdleImage;
    IosSurface   *m_switchOnImage, *m_switchOffImage;
    IosSurface   *m_radioOnImage, *m_radioOffImage;
    IosSurface   *m_upArrow, *m_downArrow, *m_leftArrow, *m_rightArrow;
    std::auto_ptr<FramePicture> m_windowFramePicture;
    std::auto_ptr<FramePicture> m_buttonIdleFramePicture;
    std::auto_ptr<FramePicture> m_buttonDownFramePicture;
    std::auto_ptr<FramePicture> m_buttonOverFramePicture;
    std::auto_ptr<FramePicture> m_textFieldIdleFramePicture;
    std::auto_ptr<FramePicture> m_separatorFramePicture;
    std::auto_ptr<FramePicture> m_listFramePicture;

    audio_manager::Sound *m_slideSound;
    audio_manager::Sound *m_whipSound, *m_whopSound;

    AbstractCursor *m_cursor;
};

extern class PuyoCommander *theCommander;

#endif // _PUYOCOMMANDER
