#ifndef _PUYOCOMMANDER
#define _PUYOCOMMANDER

#include "sofont.h"
#include "menu.h"
#include "scrollingtext.h"



typedef struct GameControlEvent {
    enum {
        kGameNone,
        kPlayer1Left,
        kPlayer1Right,
        kPlayer1TurnLeft,
        kPlayer1TurnRight,
        kPlayer1Down,
        kPlayer1DownUp,
        kPlayer2Left,
        kPlayer2Right,
        kPlayer2TurnLeft,
        kPlayer2TurnRight,
        kPlayer2Down,
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

  bool changeControlLoop(SDL_keysym *keySym);
  void controlsMenuLoop();
  void optionMenuLoop();
  void backLoop(Menu *menu);
  void startSingleGameLoop();

  void initControllers();
  void closeControllers();
  void getControlEvent(SDL_Event e, GameControlEvent *result);
  
  Menu *gameOverMenu;
  
 private:
  // SoFont *menuFont;
  // SoFont *smallFont;
  Menu *changeControlMenu;
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
};

SDL_Surface * IMG_Load_DisplayFormat (const char *path);
SDL_Surface * IMG_Load_DisplayFormatAlpha (const char *path);

extern SDL_Surface *display;
extern class PuyoCommander *theCommander;

#endif // _PUYOCOMMANDER
