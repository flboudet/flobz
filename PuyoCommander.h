#ifndef _PUYOCOMMANDER
#define _PUYOCOMMANDER

#include "sofont.h"
#include "menu.h"
#include "scrollingtext.h"
#include "GameControls.h"
#include "PuyoDoomMelt.h"

#include "ios_udpmessagebox.h"
#include "ios_igpmessagebox.h"
using namespace ios_fc;

#include "corona32.h"
#define CORONA_HEIGHT 120

class PuyoDrawable
{
  public:
    virtual void draw() = 0;
};

class PuyoNetworkGameMenuScreen;

class PuyoCommander : public MessageListener
{
 public:
  PuyoCommander(bool fullscreen,bool sound, bool audio);

  void run();
  void updateAll(PuyoDrawable *starter, SDL_Surface *extra_surf = NULL);

  void showGameOver() { menu_show(gameOverMenu); }
  void hideGameOver() { menu_hide(gameOverMenu); }

  bool changeControlLoop(int controlIndex, PuyoDrawable *starter);
  void controlsMenuLoop(PuyoDrawable *d);
  void optionMenuLoop(PuyoDrawable *d = NULL);
  void backLoop(Menu *menu, PuyoDrawable *starter = NULL);
  void startSingleGameLoop();
  void startTwoPlayerGameLoop();
  void startNetGameLoop();
  void startLANGame(int level, const char *playerName, const char *ipAddress);
  void startInternetGame(int level, const char *playerName, const char *ipAddress, int portID, int opponentIgpIdent);
  void enterStringLoop(Menu *menu, const char *kItem, char out[256], int maxlen = 10);
  void onMessage(Message &message);
  
  Menu *gameOverMenu;
  
  Menu *gameOver2PMenu;
  Menu *gameOver1PMenu;
  Menu *nextLevelMenu;
  Menu *finishedMenu;
  Menu *looserMenu;
  Menu *highScoresMenu;
  void updateHighScoresMenu(int newOne = -1);

  SoFont *smallFont;
  SoFont *darkFont;
  SoFont *menuFont;
  DoomMelt *melt;

 private:
  Menu *mainMenu;
  Menu *singleGameMenu;
  Menu *twoPlayerGameMenu;
  Menu *optionMenu;
  Menu *controlsMenu;
  Menu *rulesMenu;
  Menu *aboutMenu;
  Menu *mustRestartMenu;
  ScrollingText *scrollingText;
  unsigned int cycle, start_time;
  unsigned int lastRenderedCycle;
  unsigned int maxFrameDrop;

  static const int player1Joystick = 1;
  static const int player2Joystick = 0;
  
  int      *corona_screen;
  Corona32 *corona;
  MessageBox *mbox;
  IIM_Surface * menuselector;
  
  PuyoNetworkGameMenuScreen *networkGameMenu;
};


extern SDL_Surface *display;
extern class PuyoCommander *theCommander;

extern char *kYouDidIt;
extern char *kNextLevel;
extern char *kLooser;
extern char *kCurrentLevel;
extern char *kContinueLeft;
extern char *kGameOver;
extern char *kYouGotToLevel;
extern char *kHitActionToContinue;
extern char *kContinue;
extern char *kPlayer;
extern char *kScore;
extern char *kContinueGame;
extern char *kAbortGame;
extern char *kOptions;

#endif // _PUYOCOMMANDER
