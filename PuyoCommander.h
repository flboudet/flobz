#ifndef _PUYOCOMMANDER
#define _PUYOCOMMANDER

#include "gameui.h"
#include "Frame.h"
#include "PuyoStory.h"
#include "PuyoDataPathManager.h"
#include "PuyoLocalizedDictionary.h"
#include "GameCursor.h"
#include "NotifyCenter.h"
#include "audio.h"
#include "MainScreen.h"

using namespace gameui;

class PuyoCommander : public MessageListener, NotificationResponder
{
  public:
    PuyoCommander(String dataDir, bool fullscreen, int maxDataPackNumber=-1);
    virtual ~PuyoCommander();
    void onMessage(Message &message);

    void notificationOccured(String identifier, void * context);

    bool getMusic();
    bool getSoundFx();
    String getFullScreenKey(void) const;
    bool getFullScreen() const { return fullscreen; }
    void setFullScreen(bool fullScreen);
    bool getGlSDL() const { return useGL; }
    void setGlSDL(bool useGL);
    void setCursorVisible(bool visible) {} // TODO: Find a solution cursor->setVisible(visible); }

    const PuyoDataPathManager &getDataPathManager() { return dataPathManager; }
    const char * getLocalizedString(const char * originalString) const;

    const FramePicture *getWindowFramePicture() const { return &m_windowFramePicture; }
    const FramePicture *getButtonFramePicture() const { return &m_buttonIdleFramePicture; }
    const FramePicture *getButtonDownFramePicture() const { return &m_buttonDownFramePicture; }
    const FramePicture *getButtonOverFramePicture() const { return &m_buttonOverFramePicture; }
    const FramePicture *getEditFieldFramePicture() const { return &m_textFieldIdleFramePicture; }
    const FramePicture *getEditFieldOverFramePicture() const { return &m_textFieldIdleFramePicture; }
    const FramePicture *getSeparatorFramePicture() const { return &m_separatorFramePicture; }
    const FramePicture *getListFramePicture() const { return &m_listFramePicture; }
    IIM_Surface * getSwitchOnPicture() { return m_switchOnImage; }
    IIM_Surface * getSwitchOffPicture() { return m_switchOffImage; }
    IIM_Surface * getRadioOnPicture() { return m_radioOnImage; }
    IIM_Surface * getRadioOffPicture() { return m_radioOffImage; }
  private:

    friend class SinglePlayerGameAction;
    friend class NetGameAction;
    friend class MainMenu;
    friend class NetworkGameMenu;

    void loadPreferences(bool fs);
    void initLocale();
    void initAudio();
    void initFonts();

    PuyoDataPathManager dataPathManager;

    MessageBox *mbox;
    GameLoop   *loop;
    PuyoLocalizedDictionary * locale;

    AudioManager globalAudioManager;

    bool fullscreen;
    bool useGL;
    IosSurface   *m_frameImage, *m_buttonIdleImage, *m_buttonDownImage, *m_buttonOverImage;
    IosSurface   *m_textFieldIdleImage;
    IosSurface   *m_separatorImage;
    IosSurface   *m_listIdleImage;
    IIM_Surface   *m_switchOnImage, *m_switchOffImage;
    IIM_Surface   *m_radioOnImage, *m_radioOffImage;
    FramePicture m_windowFramePicture;
    FramePicture m_buttonIdleFramePicture, m_buttonDownFramePicture, m_buttonOverFramePicture;
    FramePicture m_textFieldIdleFramePicture;
    FramePicture m_separatorFramePicture;
    FramePicture m_listFramePicture;
};

extern class PuyoCommander *theCommander;

#endif // _PUYOCOMMANDER
