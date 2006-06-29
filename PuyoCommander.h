#ifndef _PUYOCOMMANDER
#define _PUYOCOMMANDER

#include "ios_udpmessagebox.h"
#include "ios_igpmessagebox.h"
#include "gameui.h"
#include "PuyoStory.h"
#include "PuyoDataPathManager.h"

using namespace ios_fc;
using namespace gameui;

class PuyoScreen : public Screen {
  public:
    PuyoScreen();
    virtual ~PuyoScreen() {}
    virtual void build() = 0;
};

class PuyoMainScreen : public PuyoScreen {
  public:
    PuyoMainScreen(PuyoStoryWidget *story = NULL);
    void build();
  protected:
    VBox menu;
    PuyoStoryWidget *story;
};

class PuyoMainScreenMenu;

class PuyoRealMainScreen : public PuyoScreen {
  public:
    PuyoRealMainScreen(PuyoStoryWidget *story = NULL);
    ~PuyoRealMainScreen();
    void pushMenu(PuyoMainScreenMenu *menu);
    void popMenu();
    void build() {}
    void transitionFromScreen(Screen &fromScreen);
    void onEvent(GameControlEvent *cevent);
  protected:
    SliderContainer container;
    PuyoStoryWidget *story;
    Stack<Widget*> menuStack;
    PuyoScreenTransitionWidget *transition;
};

class PuyoMainScreenMenu : public VBox {
public:
    PuyoMainScreenMenu(PuyoRealMainScreen *mainScreen, GameLoop *loop = NULL) : VBox(loop), mainScreen(mainScreen) {}
    virtual void build() = 0;
protected:
    PuyoRealMainScreen *mainScreen;
};

class PuyoPushMenuAction : public Action
{
public:
    PuyoPushMenuAction(PuyoMainScreenMenu *menu, PuyoRealMainScreen *mainScreen) : menu(menu), mainScreen(mainScreen) {}
    void action();
private:
    PuyoRealMainScreen *mainScreen;
    PuyoMainScreenMenu *menu;
};

class PuyoPopMenuAction : public Action
{
public:
    PuyoPopMenuAction(PuyoRealMainScreen *mainScreen) : mainScreen(mainScreen) {}
    void action();
private:
    PuyoRealMainScreen *mainScreen;
};

class PuyoCommander : public MessageListener
{
  public:
    PuyoCommander(String dataDir, bool fullscreen);
    void run();
    void onMessage(Message &message);

    SoFont *smallFont;
    SoFont *smallFontInfo;
    SoFont *darkFont;
    SoFont *menuFont;

    bool getMusic();
    void setMusic(bool music);
    bool getSoundFx();
    void setSoundFx(bool fx);
    bool getFullScreen() const { return fullscreen; }
    void setFullScreen(bool fullScreen);
    bool getGlSDL() const { return useGL; }
    void setGlSDL(bool useGL);
    
    void initDisplay(bool fullscreen, bool useGL);
    const PuyoDataPathManager &getDataPathManager() { return dataPathManager; }
  private:

    friend class SinglePlayerGameAction;
    friend class NetGameAction;
    friend class MainMenu;
    friend class NetworkGameMenu;

    void loadPreferences(bool fs);
    void initSDL();
    void initAudio();
    void initFonts();
    void initMenus();

    PuyoDataPathManager dataPathManager;
    
    MessageBox *mbox;
    GameLoop   *loop;

    PuyoRealMainScreen *mainScreen;

    bool fullscreen;
    bool useGL;
};


extern class PuyoCommander *theCommander;

#endif // _PUYOCOMMANDER
