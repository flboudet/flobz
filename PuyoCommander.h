#ifndef _PUYOCOMMANDER
#define _PUYOCOMMANDER

#include "sofont.h"
#include "menu.h"
#include "scrollingtext.h"



typedef struct GameControlEvent {
    enum {
        kGameNone,
        kPauseGame,
        kPlayer1Left,
        kPlayer1Right,
        kPlayer1TurnLeft,
        kPlayer1TurnRight,
        kPlayer1Down,
        kPlayer1LeftUp,
        kPlayer1RightUp,
        kPlayer1TurnLeftUp,
        kPlayer1TurnRightUp,
        kPlayer1DownUp,
        kPlayer2Left,
        kPlayer2Right,
        kPlayer2TurnLeft,
        kPlayer2TurnRight,
        kPlayer2Down,
        kPlayer2LeftUp,
        kPlayer2RightUp,
        kPlayer2TurnLeftUp,
        kPlayer2TurnRightUp,
        kPlayer2DownUp
    } gameEvent;
    enum {
        kCursorNone,
        kUp,
        kDown,
        kLeft,
        kRight,
        kStart,
        kBack,
        kQuit
    } cursorEvent;
} GameControlEvent;

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

  bool changeControlLoop(SDL_keysym *keySym, PuyoDrawable *starter);
  void controlsMenuLoop(PuyoDrawable *d);
  void optionMenuLoop(PuyoDrawable *d = NULL);
  void backLoop(Menu *menu);
  void startSingleGameLoop();

  void initControllers();
  void closeControllers();
  void getControlEvent(SDL_Event e, GameControlEvent *result);
  
  Menu *gameOverMenu;
  
  Menu *gameOver2PMenu;
  Menu *gameOver1PMenu;
  Menu *nextLevelMenu;
  Menu *finishedMenu;
  Menu *looserMenu;

 private:
  // SoFont *menuFont;
  // SoFont *smallFont;
  Menu *mainMenu;
  Menu *singleGameMenu;
  Menu *optionMenu;
  Menu *controlsMenu;
  Menu *rulesMenu;
  Menu *aboutMenu;
  ScrollingText *scrollingText;
  unsigned int cycle, start_time;
  
  SDL_Joystick *joystick[16];
  int numJoysticks;
  static const int player1Joystick = 0;
  static const int player2Joystick = 1;
  bool axis[16][16];
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
