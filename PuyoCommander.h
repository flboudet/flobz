#ifndef _PUYOCOMMANDER
#define _PUYOCOMMANDER

#include "sofont.h"
#include "menu.h"
#include "scrollingtext.h"
#include "GameControls.h"

class PuyoDrawable
{
  public:
    virtual void draw() = 0;
};

class PuyoCommander
{
 public:
  PuyoCommander(bool fullscreen,bool sound, bool audio);

  void run();
  void updateAll(PuyoDrawable *starter);

  void showGameOver() { menu_show(gameOverMenu); }
  void hideGameOver() { menu_hide(gameOverMenu); }

  bool changeControlLoop(int controlIndex, PuyoDrawable *starter);
  void controlsMenuLoop(PuyoDrawable *d);
  void optionMenuLoop(PuyoDrawable *d = NULL);
  void backLoop(Menu *menu);
  void startSingleGameLoop();
  void startTwoPlayerGameLoop();
  void enterStringLoop(Menu *menu, const char *kItem, char out[256]);

  Menu *gameOverMenu;
  
  Menu *gameOver2PMenu;
  Menu *gameOver1PMenu;
  Menu *nextLevelMenu;
  Menu *finishedMenu;
  Menu *looserMenu;

  SoFont *smallFont;
  SoFont *darkFont;
  SoFont *menuFont;

 private:
  Menu *mainMenu;
  Menu *singleGameMenu;
  Menu *twoPlayerGameMenu;
  Menu *optionMenu;
  Menu *controlsMenu;
  Menu *rulesMenu;
  Menu *highScoresMenu;
  Menu *aboutMenu;
  ScrollingText *scrollingText;
  unsigned int cycle, start_time;

  static const int player1Joystick = 1;
  static const int player2Joystick = 0;

  void updateHighScoresMenu();
};

SDL_Surface * IMG_Load_DisplayFormat (const char *path);
SDL_Surface * IMG_Load_DisplayFormatAlpha (const char *path);

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
