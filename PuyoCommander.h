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

using namespace ios_fc;
using namespace gameui;

class PuyoScreen : public Screen {
  public:
    PuyoScreen();
    virtual ~PuyoScreen() {}
    virtual void build() = 0;
};

class PuyoCommander : public MessageListener
{
  public:
    PuyoCommander(bool fullscreen, bool sound, bool audio);
    void run();

    void updateAll(DrawableComponent *starter, SDL_Surface *extra_surf = NULL);

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
    SoFont *darkFont;
    SoFont *menuFont;
    // DoomMelt *melt;

  private:

    friend class SinglePlayerGameAction;
    friend class NetGameAction;
    friend class MainMenu;
    friend class NetworkGameMenu;

    void loadPreferences(bool fs, bool snd, bool audio);
    void initSDL();
    void initAudio();
    void initFonts();
    void initDisplay();
    void initMenus();

    MessageBox *mbox;
    GameLoop   *loop;
    PuyoScreen *mainMenu, *netGameMenu;

    bool sound;
    bool fx;
    bool fullscreen;
    bool useGL;
};


extern class PuyoCommander *theCommander;

#endif // _PUYOCOMMANDER
