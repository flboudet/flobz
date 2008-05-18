#ifndef _PUYOCOMMANDER
#define _PUYOCOMMANDER

#include "ios_udpmessagebox.h"
#include "ios_igpmessagebox.h"
#include "gameui.h"
#include "Frame.h"
#include "PuyoStory.h"
#include "PuyoDataPathManager.h"
#include "PuyoLocalizedDictionary.h"
#include "GameCursor.h"
#include "NotifyCenter.h"
#include "audio.h"

using namespace ios_fc;
using namespace gameui;

class PuyoScreen : public Screen {
  public:
    PuyoScreen();
    virtual ~PuyoScreen() {}
    virtual void build() = 0;
};

class PuyoMainScreenMenu;

class PuyoMainScreen : public PuyoScreen, public SliderContainerListener {
  public:
    PuyoMainScreen(PuyoStoryWidget *fgStory = NULL, PuyoStoryWidget *bgStory = NULL);
    ~PuyoMainScreen();
    void pushMenu(PuyoMainScreenMenu *menu, bool fullScreen = false);
    void popMenu();
    void build() {}
    void transitionFromScreen(Screen &fromScreen);
    void onEvent(GameControlEvent *cevent);
    // SliderContainerListener implementation
    /**
     * Notify that the slider is outside of the screen, before sliding back inside
     */
    virtual void onSlideOutside(SliderContainer &slider);
  protected:
    void setMenuDimensions();
    SliderContainer container;
    PuyoStoryWidget *fgStory, *bgStory;
    Stack<Widget*> menuStack;
    Stack<bool> fullScreenStack;
    PuyoScreenTransitionWidget *transition;
    bool nextFullScreen;
};

class PuyoMainScreenMenu : public Frame {
public:
    PuyoMainScreenMenu(PuyoMainScreen *mainScreen, GameLoop *loop = NULL);
    virtual void build() = 0;
    // Notifications
    virtual void onMainScreenMenuPushed() {}
    virtual void onMainScreenMenuPoped() {}
protected:
    PuyoMainScreen *mainScreen;
};

class PuyoPushMenuAction : public Action
{
public:
    PuyoPushMenuAction(PuyoMainScreenMenu *menu, PuyoMainScreen *mainScreen, bool fullScreen = false)
      : mainScreen(mainScreen), menu(menu), m_fullScreen(fullScreen) {}
    void action();
private:
    PuyoMainScreen *mainScreen;
    PuyoMainScreenMenu *menu;
    bool m_fullScreen;
};

class PuyoPopMenuAction : public Action
{
public:
    PuyoPopMenuAction(PuyoMainScreen *mainScreen) : mainScreen(mainScreen) {}
    void action();
private:
    PuyoMainScreen *mainScreen;
};

class PuyoCommander : public MessageListener, NotificationResponder
{
  public:
    PuyoCommander(String dataDir, bool fullscreen, int maxDataPackNumber=-1);
    virtual ~PuyoCommander();
    void run();
    void debug_gsl(String gsl_script);
    void onMessage(Message &message);

    SoFont *smallFont;
    SoFont *smallFontInfo;
    SoFont *darkFont;
    SoFont *menuFont;
    SoFont *funnyFont;

    void notificationOccured(String identifier, void * context);
    
    bool getMusic();
    bool getSoundFx();
    String getFullScreenKey(void) const;
    bool getFullScreen() const { return fullscreen; }
    void setFullScreen(bool fullScreen);
    bool getGlSDL() const { return useGL; }
    void setGlSDL(bool useGL);
    void setCursorVisible(bool visible) { cursor->setVisible(visible); }
    
    void initDisplay(int w, int h, bool fullscreen, bool useGL);
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
  private:

    friend class SinglePlayerGameAction;
    friend class NetGameAction;
    friend class MainMenu;
    friend class NetworkGameMenu;

    void loadPreferences(bool fs);
    void initSDL();
    void initLocale();
    void initAudio();
    void initFonts();
    void initMenus();

    PuyoDataPathManager dataPathManager;
    
    MessageBox *mbox;
    GameLoop   *loop;

    PuyoMainScreen *mainScreen;
    GameCursor *cursor;
    PuyoLocalizedDictionary * locale;
    
    AudioManager globalAudioManager;

    bool fullscreen;
    bool useGL;
    IIM_Surface   *m_frameImage, *m_buttonIdleImage, *m_buttonDownImage, *m_buttonOverImage;
    IIM_Surface   *m_textFieldIdleImage;
    IIM_Surface   *m_separatorImage;
    IIM_Surface   *m_listIdleImage;
    IIM_Surface   *m_switchOnImage, *m_switchOffImage;
    FramePicture m_windowFramePicture;
    FramePicture m_buttonIdleFramePicture, m_buttonDownFramePicture, m_buttonOverFramePicture;
    FramePicture m_textFieldIdleFramePicture;
    FramePicture m_separatorFramePicture;
    FramePicture m_listFramePicture;
};

extern class PuyoCommander *theCommander;

#endif // _PUYOCOMMANDER
