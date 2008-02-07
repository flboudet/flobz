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

class PuyoCommander : public MessageListener
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

    bool getMusic();
    void setMusic(bool music);
    bool getSoundFx();
    void setSoundFx(bool fx);
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

    bool fullscreen;
    bool useGL;
    IIM_Surface   *m_frameImage, *m_buttonIdleImage, *m_buttonDownImage;
    FramePicture m_windowFramePicture;
    FramePicture m_buttonIdleFramePicture, m_buttonDownFramePicture, m_buttonOverFramePicture;
};

extern class PuyoCommander *theCommander;

#endif // _PUYOCOMMANDER
