#ifndef _PUYOCOMMANDER
#define _PUYOCOMMANDER

/*
#include "sofont.h"
#include "menu.h"
#include "scrollingtext.h"
#include "GameControls.h"
#include "PuyoDoomMelt.h"
*/

#include "ios_udpmessagebox.h"
#include "ios_igpmessagebox.h"
#include "gameui.h"
#include "PuyoStory.h"

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
    PuyoCommander(bool fullscreen, bool sound, bool audio);
    void run();

    /*
       bool changeControlLoop(int controlIndex, PuyoDrawable *starter);
       void controlsMenuLoop(PuyoDrawable *d);
       void optionMenuLoop(PuyoDrawable *d = NULL);
       void backLoop(Menu *menu, PuyoDrawable *starter = NULL);
       void startSingleGameLoop();
       void startTwoPlayerGameLoop();
       void startNetGameLoop();
       void startLANGame(int level, const char *playerName, const char *ipAddress);
       void startInternetGame(int level, const char *playerName, const char *ipAddress, int portID, int playerIgpIdent, int opponentIgpIdent);
       void enterStringLoop(Menu *menu, const char *kItem, char out[256], int maxlen = 10);
       */
    void onMessage(Message &message);
    // void updateHighScoresMenu(int newOne = -1);

    SoFont *smallFont;
    SoFont *smallFontInfo;
    SoFont *darkFont;
    SoFont *menuFont;
    // DoomMelt *melt;

    bool getMusic() const { return sound; }
    void setMusic(bool music);
    bool getSoundFx() const { return fx; }
    void setSoundFx(bool fx);
    bool getFullScreen() const { return fullscreen; }
    void setFullScreen(bool fullScreen);
    bool getGlSDL() const { return useGL; }
    void setGlSDL(bool useGL);
    
    void initDisplay(bool fullscreen, bool useGL);
  private:

    friend class SinglePlayerGameAction;
    friend class NetGameAction;
    friend class MainMenu;
    friend class NetworkGameMenu;

    void loadPreferences(bool fs, bool snd, bool audio);
    void initSDL();
    void initAudio();
    void initFonts();
    void initMenus();

    MessageBox *mbox;
    GameLoop   *loop;

    PuyoRealMainScreen *mainScreen;

    bool sound;
    bool fx;
    bool fullscreen;
    bool useGL;
};


extern class PuyoCommander *theCommander;

#endif // _PUYOCOMMANDER
