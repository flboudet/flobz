#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "glSDL.h"
#include "PuyoCommander.h"
#include "PuyoStarter.h"
#include "PuyoVersion.h"
#include "PuyoStory.h"
#include "preferences.h"
#include "InputManager.h"
#include "HiScores.h"
#include "PuyoDoomMelt.h"
#include "IosImgProcess.h"

#ifndef DATADIR
extern char *DATADIR;
#endif

extern const char *p1name;
extern const char *p2name;
extern int GAME_ACCEL;
extern int gameLevel;

char *kYouDidIt = "You Dit It!!!";
char *kNextLevel = "Next:\t\t";
char *kLooser = "Looser!!!";
char *kCurrentLevel = "Level:\t\t";
char *kContinueLeft = "Continues:\t\t";
char *kGameOver = "Game Over!!!";
char *kYouGotToLevel = "You Get To ";
char *kHitActionToContinue = "Hit Action to continue...";
char *kHitActionForMenu = "Hit Action for menu...";
char *kContinue    = "Continue?";
char *kContinueGame= "Continue Game";
char *kAbortGame   = "Abort Game";
char *kOptions     = "Options";
char *kPlayer      = "Player";
char *kScore       = "Score:\t";
char *kPlayerName  = "Player Name:\t";
char *kPlayer1Name  = "P1 Name:\t";
char *kPlayer2Name  = "P2 Name:\t";


static const char *k01 = " 1 - ";
static const char *k02 = " 2 - ";
static const char *k03 = " 3 - ";
static const char *k04 = " 4 - ";
static const char *k05 = " 5 - ";
static const char *k06 = " 6 - ";
static const char *k07 = " 7 - ";
static const char *k08 = " 8 - ";
static const char *k09 = " 9 - ";
static const char *k10 = "10 - ";

static const char *kMustRestart1 = "Major Video Mode Change...";
static const char *kMustRestart2 = "This will take effect next time you restart Flobo Puyo.";
char *kHighScores  = "Hall of Fame";

static char *kMusicVolume = "MusicVolume";
static char *kAudioVolume = "AudioVolume";

char *kCongratulations = "Congratulations!!!";
char *kPuyosInvasion   = "                        You stopped Puyo's invasion.\n"
                         "                         Peace on Earth is restored!!";

static char *kAudioFX     = "Audio FX\t";
static char *kMusic       = "Music\t";
static char *kFullScreen  = "FullScreen\t";
static char *kOpenGL      = "Use OpenGL\t";
static char *kControls    = "Change controls...";
static char *kGameLevel   = "Choose Game Level";
static char *kLevelEasy   = "Easy";
static char *kLevelMedium = "Medium";
static char *kLevelHard   = "Hard";

static char *kPlayer1Left  = "P2 Left:\t";
static char *kPlayer1Right = "P2 Right:\t";
static char *kPlayer1Down  = "P2 Down:\t";
static char *kPlayer1Clockwise        = "P2 Turn L:\t";
static char *kPlayer1Counterclockwise = "P2 Turn R:\t";

static char *kPlayer2Left   = "P1 Left:\t";
static char *kPlayer2Right  = "P1 Right:\t";
static char *kPlayer2Down   = "P1 Down:\t";
static char *kPlayer2Clockwise        = "P1 Turn L:\t";
static char *kPlayer2Counterclockwise = "P1 Turn R:\t";

static char *kRules01 = "Puyos are fancy smiling bubbles...";
static char *kRules02 = "But they can really be invading sometimes!";
static char *kRules03 = "Send them away by making groups of 4 or more.";
static char *kRules04 = "Try to make large groups, or many at the same time";
static char *kRules05 = "to send more bad ghost Puyos to your opponent.";

static char *kAbout01 = "FloboPuyo is an iOS-software production.\t\t";
static char *kAbout02 = "Puyo's world Meta God:\t\t";
static char *kAbout03 = "\t\tFlorent 'flobo' Boudet";
static char *kAbout04 = "X Men:\t\t";
static char *kAbout05 = "\t\tJean-Christophe 'jeko' Hoelt";
static char *kAbout06 = "\t\tGuillaume 'gyom' Borios";
static char *kAbout07 = "Beta Goddess:\t\t";
static char *kAbout08 = "\t\tTania";

char *AI_NAMES[] = { "Fanzy", "Garou", "Big Rabbit", "Gizmo",
  "Satanas", "Doctor X", "Tanya", "Mr Gyom",
  "The Duke","Jeko","--------" };


extern SDL_Surface *display;
extern IIM_Surface *image, *gameScreen;
IIM_Surface *menuBGImage = 0;
PuyoCommander *theCommander;

const int cycle_duration = 20;

extern bool fullscreen;
extern bool useGL;
static bool sound = true;
static bool fx = true;

#define SINGLE_PLAYER_GAME "Single Player Game"
#define TWO_PLAYERS_GAME "Two Players Game"

MenuItems
main_menu_load (SoFont * font)
{
  static MenuItemsTab main_menu = {
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM (SINGLE_PLAYER_GAME),
    MENUITEM (TWO_PLAYERS_GAME),
    MENUITEM_BLANKLINE,
    MENUITEM ("Options"),
    MENUITEM_BLANKLINE,
    MENUITEM (kHighScores),
    MENUITEM_BLANKLINE,
    MENUITEM ("Rules"),
    MENUITEM ("About FloboPuyo"),
    MENUITEM_BLANKLINE,
    MENUITEM ("Quit"),
    MENUITEM_END
  };
  static int loaded = 0;

  if (!loaded) {
    menu_items_set_font_for (main_menu, SINGLE_PLAYER_GAME, font);
    menu_items_set_font_for (main_menu, TWO_PLAYERS_GAME, font);
    menu_items_set_font_for (main_menu, kHighScores, font);
    menu_items_set_font_for (main_menu, "Options", font);
    menu_items_set_font_for (main_menu, "Rules", font);
    menu_items_set_font_for (main_menu, "About FloboPuyo", font);
    menu_items_set_font_for (main_menu, "Quit", font);
    loaded = 1;
    menuBGImage = IIM_Load_DisplayFormat("MenuBackground.jpg");
  }

  return main_menu;
}


MenuItems two_player_game_menu_load (SoFont *font, SoFont *small_font)
{
  static MenuItemsTab go_menu =
  {
    MENUITEM(kPlayer1Name),
    MENUITEM(kPlayer2Name),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kGameLevel),
    MENUITEM_BLANKLINE,
    MENUITEM(kLevelEasy),
    MENUITEM(kLevelMedium),
    MENUITEM(kLevelHard),
    MENUITEM_END
  };
  menu_items_set_font_for(go_menu, kGameLevel,   font);
  menu_items_set_font_for(go_menu, kLevelEasy,   font);
  menu_items_set_font_for(go_menu, kLevelMedium, font);
  menu_items_set_font_for(go_menu, kLevelHard,   font);
  menu_items_set_font_for(go_menu, kPlayer1Name, font);
  menu_items_set_font_for(go_menu, kPlayer2Name, font);
  return go_menu;
}

MenuItems single_game_menu_load (SoFont *font, SoFont *small_font)
{
  static MenuItemsTab go_menu =
  {
    MENUITEM(kPlayerName),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kGameLevel),
    MENUITEM_BLANKLINE,
    MENUITEM(kLevelEasy),
    MENUITEM(kLevelMedium),
    MENUITEM(kLevelHard),
    MENUITEM_END
  };
  menu_items_set_font_for(go_menu, kGameLevel,   font);
  menu_items_set_font_for(go_menu, kLevelEasy,   font);
  menu_items_set_font_for(go_menu, kLevelMedium, font);
  menu_items_set_font_for(go_menu, kLevelHard,   font);
  menu_items_set_font_for(go_menu, kPlayerName,  font);
  return go_menu;
}

MenuItems gameover_2p_menu_load (SoFont *font, SoFont *small_font)
{
  static MenuItemsTab go_menu = {
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kPlayer),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kScore),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kContinue),
    MENUITEM("YES"),
    MENUITEM("NO"),
    MENUITEM_END
  };
  menu_items_set_font_for(go_menu, kPlayer,    font);
  menu_items_set_font_for(go_menu, kScore,     font);
  menu_items_set_font_for(go_menu, kContinue,  font);
  return go_menu;
}

MenuItems finished_1p_menu_load (SoFont *font, SoFont *small_font)
{
  static MenuItemsTab go_menu = {
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kCongratulations),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kPuyosInvasion),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kHitActionForMenu),
    MENUITEM("YES"),
    MENUITEM("NO"),
    MENUITEM_END
  };
  menu_items_set_font_for(go_menu, kCongratulations,  font);
  menu_items_set_font_for(go_menu, kPuyosInvasion, font);
  menu_items_set_font_for(go_menu, kHitActionForMenu, small_font);
  return go_menu;
}

MenuItems nextlevel_1p_menu_load (SoFont *font, SoFont *small_font)
{
  static MenuItemsTab go_menu = {
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kYouDidIt),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kNextLevel),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kHitActionToContinue),
    MENUITEM("YES"),
    MENUITEM("NO"),
    MENUITEM_END
  };
  menu_items_set_font_for(go_menu, kYouDidIt,  font);
  menu_items_set_font_for(go_menu, kNextLevel, font);
  menu_items_set_font_for(go_menu, kHitActionToContinue, small_font);
  return go_menu;
}

MenuItems looser_1p_menu_load (SoFont *font, SoFont *small_font)
{
  static MenuItemsTab go_menu = {
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kLooser),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kCurrentLevel),
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kContinueLeft),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kHitActionToContinue),
    MENUITEM("YES"),
    MENUITEM("NO"),
    MENUITEM_END
  };
  menu_items_set_font_for(go_menu, kLooser,  font);
  menu_items_set_font_for(go_menu, kCurrentLevel, font);
  menu_items_set_font_for(go_menu, kContinueLeft, font);
  menu_items_set_font_for(go_menu, kHitActionToContinue, small_font);
  return go_menu;
}

MenuItems gameover_1p_menu_load (SoFont *font, SoFont *small_font)
{
  static MenuItemsTab go_menu = {
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kGameOver),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kYouGotToLevel),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kHitActionForMenu),
    MENUITEM("YES"),
    MENUITEM("NO"),
    MENUITEM_END
  };
  menu_items_set_font_for(go_menu, kGameOver,    font);
  menu_items_set_font_for(go_menu, kYouGotToLevel, font);
  menu_items_set_font_for(go_menu, kHitActionForMenu,  small_font);
  return go_menu;
}

MenuItems rules_menu_load (SoFont *font) {
  static MenuItemsTab option_menu = {
    MENUITEM_INACTIVE(kRules01),
    MENUITEM_INACTIVE(kRules02),
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kRules03),
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kRules04),
    MENUITEM_INACTIVE(kRules05),
    MENUITEM_BLANKLINE,
    MENUITEM("Back"),
    MENUITEM_END
  };
  menu_items_set_font_for(option_menu, kRules01, font);
  menu_items_set_font_for(option_menu, kRules02, font);
  menu_items_set_font_for(option_menu, kRules03, font);
  menu_items_set_font_for(option_menu, kRules04, font);
  menu_items_set_font_for(option_menu, kRules05, font);

  return option_menu;
}

MenuItems high_scores_menu_load (SoFont *font)
{
  static MenuItemsTab option_menu =
  {
    MENUITEM_INACTIVE(kHighScores),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(k01),
    MENUITEM_INACTIVE(k02),
    MENUITEM_INACTIVE(k03),
    MENUITEM_INACTIVE(k04),
    MENUITEM_INACTIVE(k05),
    MENUITEM_INACTIVE(k06),
    MENUITEM_INACTIVE(k07),
    MENUITEM_INACTIVE(k08),
    MENUITEM_INACTIVE(k09),
    MENUITEM_INACTIVE(k10),
    MENUITEM("Back"),
    MENUITEM_END
  };
  menu_items_set_font_for(option_menu, kHighScores, font);
  menu_items_set_font_for(option_menu, k01, font);
  menu_items_set_font_for(option_menu, k02, font);
  menu_items_set_font_for(option_menu, k03, font);
  menu_items_set_font_for(option_menu, k04, font);
  menu_items_set_font_for(option_menu, k05, font);
  menu_items_set_font_for(option_menu, k06, font);
  menu_items_set_font_for(option_menu, k07, font);
  menu_items_set_font_for(option_menu, k08, font);
  menu_items_set_font_for(option_menu, k09, font);
  menu_items_set_font_for(option_menu, k10, font);

  return option_menu;
}

MenuItems about_menu_load (SoFont *font)
{
  static MenuItemsTab option_menu = {
    MENUITEM_INACTIVE(kAbout01),
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kVersion),
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kAbout02),
    MENUITEM_INACTIVE(kAbout03),
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kAbout04),
    MENUITEM_INACTIVE(kAbout05),
    MENUITEM_INACTIVE(kAbout06),
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kAbout07),
    MENUITEM_INACTIVE(kAbout08),
    MENUITEM_BLANKLINE,
    MENUITEM("Back"),
    MENUITEM_END
  };
  menu_items_set_font_for(option_menu, kAbout01, font);
  menu_items_set_font_for(option_menu, kVersion, font);
  menu_items_set_font_for(option_menu, kAbout02, font);
  menu_items_set_font_for(option_menu, kAbout03, font);
  menu_items_set_font_for(option_menu, kAbout04, font);
  menu_items_set_font_for(option_menu, kAbout05, font);
  menu_items_set_font_for(option_menu, kAbout06, font);
  menu_items_set_font_for(option_menu, kAbout07, font);
  menu_items_set_font_for(option_menu, kAbout08, font);

  return option_menu;
}

MenuItems must_restart_menu_load (SoFont *font)
{
  static MenuItemsTab option_menu = {
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kMustRestart1),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kMustRestart2),
    MENUITEM_BLANKLINE,
    MENUITEM("Back"),
    MENUITEM_END
  };
  menu_items_set_font_for(option_menu, kMustRestart1, font);
  menu_items_set_font_for(option_menu, kMustRestart2, font);
  return option_menu;
}

MenuItems options_menu_load (SoFont *font, SoFont *small_font)
{
  static MenuItemsTab option_menu = {
    MENUITEM(kFullScreen),
#ifdef HAVE_OPENGL
    MENUITEM(kOpenGL),
#endif
    MENUITEM_BLANKLINE,
    MENUITEM(kMusic),
    MENUITEM(kAudioFX),
    MENUITEM_BLANKLINE,
    MENUITEM(kControls),
    MENUITEM_BLANKLINE,
    MENUITEM("Back"),
    MENUITEM_END
  };
  menu_items_set_font_for(option_menu,  kMusic, font);
  menu_items_set_font_for(option_menu,  kAudioFX, font);
  menu_items_set_font_for(option_menu,  kControls, font);
  menu_items_set_font_for(option_menu,  "Back", font);
  menu_items_set_font_for(option_menu,  kFullScreen, font);
  menu_items_set_value_for(option_menu, kFullScreen, fullscreen?"ON":"OFF");
#ifdef HAVE_OPENGL
  menu_items_set_font_for(option_menu,  kOpenGL, font);
  menu_items_set_value_for(option_menu, kOpenGL, useGL?"ON":"OFF");
#endif
  menu_items_set_value_for(option_menu, kMusic,      sound?"ON":"OFF");
  menu_items_set_value_for(option_menu, kAudioFX,    fx?"ON":"OFF");
  return option_menu;
}

MenuItems controls_menu_load (SoFont *font, SoFont *small_font)
{
  static MenuItemsTab controls_menu = {
    //MENUITEM("Player 1 Joystick"),
    //MENUITEM("Player 2 Joystick"),
    MENUITEM_BLANKLINE,
    MENUITEM(kPlayer2Left),
    MENUITEM(kPlayer2Right),
    MENUITEM(kPlayer2Down),
    MENUITEM(kPlayer2Clockwise),
    MENUITEM(kPlayer2Counterclockwise),
    MENUITEM_BLANKLINE,
    MENUITEM(kPlayer1Left),
    MENUITEM(kPlayer1Right),
    MENUITEM(kPlayer1Down),
    MENUITEM(kPlayer1Clockwise),
    MENUITEM(kPlayer1Counterclockwise),
    MENUITEM_BLANKLINE,
    MENUITEM("Back"),
    MENUITEM_END
  };
  //menu_items_set_font_for(controls_menu,  "Player 1 Joystick", font);
  
  menu_items_set_font_for(controls_menu,  kPlayer1Left, font);
  menu_items_set_font_for(controls_menu,  kPlayer1Right, font);
  menu_items_set_font_for(controls_menu,  kPlayer1Down, font);
  menu_items_set_font_for(controls_menu,  kPlayer1Clockwise, font);
  menu_items_set_font_for(controls_menu,  kPlayer1Counterclockwise, font);
  
  menu_items_set_font_for(controls_menu,  kPlayer2Left, font);
  menu_items_set_font_for(controls_menu,  kPlayer2Right, font);
  menu_items_set_font_for(controls_menu,  kPlayer2Down, font);
  menu_items_set_font_for(controls_menu,  kPlayer2Clockwise, font);
  menu_items_set_font_for(controls_menu,  kPlayer2Counterclockwise, font);
  
  menu_items_set_font_for(controls_menu,  "Back", font);
 //menu_items_set_value_for(controls_menu, "Player 1 Joystick", SDL_JoystickName(0));
  //menu_items_set_value_for(controls_menu, kPlayer1Left, "s");
  return controls_menu;
}

Menu *menu_pause = NULL;

MenuItems pause_menu_load (SoFont * font)
{
  static MenuItemsTab main_menu = {
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM (kContinueGame),
    MENUITEM_BLANKLINE,
    MENUITEM (kOptions),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM (kAbortGame),
    MENUITEM_END
  };
  static int loaded = 0;

  if (!loaded) {
    menu_items_set_font_for (main_menu, kContinueGame, font);
    menu_items_set_font_for (main_menu, kOptions, font);
    menu_items_set_font_for (main_menu, kAbortGame, font);
    loaded = 1;
  }

  return main_menu;
}

PuyoCommander::PuyoCommander(bool fs, bool snd, bool audio)
{
  int init_flags = SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK;

  SDL_Delay(500);
  corona = NULL;
  fullscreen = GetBoolPreference(kFullScreen,fs);
#ifdef HAVE_OPENGL
  useGL      = GetBoolPreference(kOpenGL,false);
#endif
  sound = GetBoolPreference(kMusic,snd);
  fx = GetBoolPreference(kAudioFX,audio);

  int music_volume = GetIntPreference(kMusicVolume, 100);
  int audio_volume = GetIntPreference(kAudioVolume, 80);

  initGameControls();
#ifdef USE_DGA
  /* This Hack Allows Hardware Surface on Linux */
  if (fullscreen)
    setenv("SDL_VIDEODRIVER","dga",0);

  if (SDL_Init(init_flags) < 0) {
    setenv("SDL_VIDEODRIVER","x11",1);
    if (SDL_Init(init_flags) < 0) {
      fprintf(stderr, "SDL initialisation error:  %s\n", SDL_GetError());
      exit(1);
    }
  }
  else {
    if (fullscreen)
      SDL_WM_GrabInput(SDL_GRAB_ON);
  }
#else
#ifdef WIN32
  _putenv("SDL_VIDEODRIVER=windib");
#endif
  if ( SDL_Init(init_flags) < 0 ) {
    fprintf(stderr, "SDL initialisation error:  %s\n", SDL_GetError());
    exit(1);
  }
#endif

  initControllers();
  initHiScores(AI_NAMES);
  
#ifdef USE_AUDIO
  audio_init();
  audio_music_start(0);
  if (sound==false) Mix_PauseMusic();
  audio_set_music_on_off(sound);
  audio_set_sound_on_off(fx);

  audio_set_volume(audio_volume);
  audio_music_set_volume(music_volume);
#endif

  display = SDL_SetVideoMode( 640, 480, 0,  SDL_ANYFORMAT|SDL_HWSURFACE|SDL_DOUBLEBUF|(fullscreen?SDL_FULLSCREEN:0)|(useGL?SDL_GLSDL:0));
  if ( display == NULL ) {
    fprintf(stderr, "SDL_SetVideoMode error: %s\n",
            SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);
  SDL_ShowCursor(SDL_DISABLE);

  smallFont = SoFont_new();
  SoFont_load (smallFont, IIM_Load_DisplayFormatAlpha ("font4b.png"));
  menuFont = SoFont_new();
  SoFont_load (menuFont, IIM_Load_DisplayFormatAlpha ("font3b.png"));
  darkFont = SoFont_new();
  SoFont_load (darkFont, IIM_Load_DisplayFormatAlpha ("fontdark.png"));

  IIM_Surface * menuselector = IIM_Load_DisplayFormatAlpha("menusel.png");

  mainMenu = menu_new(main_menu_load(menuFont),menuselector);
  gameOver1PMenu = menu_new(gameover_1p_menu_load(menuFont, smallFont),menuselector);
  gameOver2PMenu = menu_new(gameover_2p_menu_load(menuFont, smallFont),menuselector);
  nextLevelMenu  = menu_new(nextlevel_1p_menu_load(menuFont, smallFont),menuselector);
  looserMenu     = menu_new(looser_1p_menu_load(menuFont, smallFont),menuselector);
  finishedMenu   = menu_new(finished_1p_menu_load(menuFont, smallFont),menuselector);
  gameOverMenu   = gameOver2PMenu;
  optionMenu     = menu_new(options_menu_load(menuFont, smallFont),menuselector);
  controlsMenu   = menu_new(controls_menu_load(menuFont, smallFont),menuselector);
  rulesMenu      = menu_new(rules_menu_load(menuFont),menuselector);
  highScoresMenu = menu_new(high_scores_menu_load(menuFont),menuselector);
  aboutMenu      = menu_new(about_menu_load(menuFont),menuselector);
  singleGameMenu    = menu_new(single_game_menu_load(menuFont,smallFont),menuselector);
  twoPlayerGameMenu = menu_new(two_player_game_menu_load(menuFont,smallFont),menuselector);
  mustRestartMenu   = menu_new(must_restart_menu_load(menuFont),menuselector); 

  if (menu_pause == NULL) menu_pause = menu_new(pause_menu_load(menuFont),menuselector);

  menu_set_sounds (optionMenu,     sound_pop, sound_slide);
  menu_set_sounds (controlsMenu,   sound_pop, sound_slide);
  menu_set_sounds (mainMenu,       sound_pop, sound_slide);
  menu_set_sounds (rulesMenu,      sound_pop, sound_slide);
  menu_set_sounds (highScoresMenu, sound_pop, sound_slide);
  menu_set_sounds (aboutMenu,      sound_pop, sound_slide);
  menu_set_sounds (singleGameMenu, sound_pop, sound_slide);
  menu_set_sounds (twoPlayerGameMenu, sound_pop, sound_slide);
  menu_set_sounds (menu_pause    , sound_pop, sound_slide);
  melt = doom_melt_new();

  scrollingText = scrolling_text_new(
    "Welcome to the wonderful world of FloboPuyo !!!  Enjoy its nice graphics, "
    "happy music and entertaining gameplay...  "
    "Will you be able to defeat all of the mighty players ?  "
    "Will you beat the Puyo Gods ???  Have a try !  "
    "We wish you good luck.                                                    "
    "                                Hello from PuyoLand !", smallFont);
  theCommander = this;
}

void PuyoCommander::run()
{
    Menu   *menu = mainMenu;
    
    cycle = 0;
    lastRenderedCycle = 0;
    maxFrameDrop = 10;
    
    start_time = SDL_GetTicks ();
    
    audio_music_start (0);
    
    /*  PuyoStory *introStory = new PuyoStory(this, 0);
    introStory->loop();
    delete introStory; */
    
    menu_show (menu);
    
    while (1) {
        SDL_Event e;
        
        while (SDL_PollEvent (&e)) {
            GameControlEvent controlEvent;
            getControlEvent(e, &controlEvent);
            
            switch (controlEvent.cursorEvent) {
                case GameControlEvent::kQuit:
                    goto mml_fin;
                    break;
                case GameControlEvent::kDown:
                    menu_next_item (menu);
                    break;
                case GameControlEvent::kUp:
                    menu_prev_item (menu);
                    break;
                case GameControlEvent::kStart:
                    menu_validate (menu);
                    if (menu_active_is (menu, "Quit"))
                        goto mml_fin;
                    if (menu_active_is (menu, "Options")) {
                        menu_hide (menu);
                        optionMenuLoop(NULL);
                        menu_show (menu);
                    }
                    if (menu_active_is (menu, kHighScores)) {
                        menu_hide(menu);
                        updateHighScoresMenu();
                        backLoop(highScoresMenu);
                        menu_show(menu);
                    }
                    if (menu_active_is (menu, "Rules")) {
                        menu_hide(menu);
                        backLoop(rulesMenu);
                        menu_show(menu);
                    }
                    if (menu_active_is (menu, "About FloboPuyo")) {
                        menu_hide(menu);
                        backLoop(aboutMenu);
                        menu_show(menu);
                    }
                    if (menu_active_is (menu, SINGLE_PLAYER_GAME)) {
                        menu_hide (menu);
                        startSingleGameLoop();
                        menu_show (menu);
                    }
                    if (menu_active_is (menu, TWO_PLAYERS_GAME)) {
                        menu_hide (menu);
                        startTwoPlayerGameLoop();
                        menu_show (menu);
                        audio_music_start(0);
                    }
                    break;
                case GameControlEvent::kBack:
                    goto mml_fin;
                    break;
                default:
                    break;
            }
        }
    updateAll(NULL);
  }
mml_fin:
  menu_hide (menu);
}
  
void PuyoCommander::updateHighScoresMenu(int newOne)
{
  hiscore *scores = getHiScores();
  char tmp[256];
#define PAS_DE_COMMENTAIRES(X,kXX) \
  if (newOne == X)  \
    sprintf(tmp, "%s\t** %d", scores[X].name, scores[X].score); \
  else \
    sprintf(tmp, "%s\t%d", scores[X].name, scores[X].score); \
  menu_set_value(highScoresMenu,kXX,tmp,0);

  PAS_DE_COMMENTAIRES(0,k01);
  PAS_DE_COMMENTAIRES(1,k02);
  PAS_DE_COMMENTAIRES(2,k03);
  PAS_DE_COMMENTAIRES(3,k04);
  PAS_DE_COMMENTAIRES(4,k05);
  PAS_DE_COMMENTAIRES(5,k06);
  PAS_DE_COMMENTAIRES(6,k07);
  PAS_DE_COMMENTAIRES(7,k08);
  PAS_DE_COMMENTAIRES(8,k09);
  PAS_DE_COMMENTAIRES(9,k10);
}

bool PuyoCommander::changeControlLoop(int controlIndex, PuyoDrawable *starter)
{
  bool keyPressed = false;
  while (1) {
    SDL_Event e;

    while (SDL_PollEvent (&e)) {
      GameControlEvent controlEvent;
      bool tryOk = tryChangeControl(controlIndex, e, &controlEvent);

      switch (controlEvent.cursorEvent) {
        case GameControlEvent::kQuit:
          exit(0);
          goto mml_fin;
          break;
        case GameControlEvent::kBack:
          goto mml_fin;
          break;
        default:
          break;
      }
      if (tryOk) {
        keyPressed = true;
        goto mml_fin;
      }
    }
    updateAll(starter);
  }
mml_fin:
  return keyPressed;
}

void PuyoCommander::controlsMenuLoop(PuyoDrawable *d)
{
  char newKeyName[250];

  getKeyName(kPlayer1LeftControl, newKeyName);
  menu_set_value(controlsMenu, kPlayer1Left, newKeyName,0);
  getKeyName(kPlayer1RightControl, newKeyName);
  menu_set_value(controlsMenu, kPlayer1Right, newKeyName,0);
  getKeyName(kPlayer1DownControl, newKeyName);
  menu_set_value(controlsMenu, kPlayer1Down, newKeyName,0);
  getKeyName(kPlayer1ClockwiseControl, newKeyName);
  menu_set_value(controlsMenu, kPlayer1Clockwise, newKeyName,0);
  getKeyName(kPlayer1CounterclockwiseControl, newKeyName);
  menu_set_value(controlsMenu, kPlayer1Counterclockwise, newKeyName,0);

  getKeyName(kPlayer2LeftControl, newKeyName);
  menu_set_value(controlsMenu, kPlayer2Left, newKeyName,0);
  getKeyName(kPlayer2RightControl, newKeyName);
  menu_set_value(controlsMenu, kPlayer2Right, newKeyName,0);
  getKeyName(kPlayer2DownControl, newKeyName);
  menu_set_value(controlsMenu, kPlayer2Down, newKeyName,0);
  getKeyName(kPlayer2ClockwiseControl, newKeyName);
  menu_set_value(controlsMenu, kPlayer2Clockwise, newKeyName,0);
  getKeyName(kPlayer2CounterclockwiseControl, newKeyName);
  menu_set_value(controlsMenu, kPlayer2Counterclockwise, newKeyName,1);

  menu_show(controlsMenu);
  while (1) {
    SDL_Event e;

    while (SDL_PollEvent (&e)) {
      char *chosenControl = NULL;
      int chosenControlIndex;
      GameControlEvent controlEvent;
      getControlEvent(e, &controlEvent);

      switch (controlEvent.cursorEvent) {
        case GameControlEvent::kQuit:
          exit(0);
          goto mml_fin;
          break;
        case GameControlEvent::kUp:
          menu_prev_item (controlsMenu);
          break;
        case GameControlEvent::kDown:
          menu_next_item (controlsMenu);
          break;
        case GameControlEvent::kStart:
          menu_validate (controlsMenu);
          if (menu_active_is (controlsMenu, "Back"))
            goto mml_fin;

          if (menu_active_is (controlsMenu, kPlayer1Left)) {
            chosenControl = kPlayer1Left;
            chosenControlIndex = kPlayer1LeftControl;
          }
          if (menu_active_is (controlsMenu, kPlayer1Right)) {
            chosenControl = kPlayer1Right;
            chosenControlIndex = kPlayer1RightControl;
          }
          if (menu_active_is (controlsMenu, kPlayer1Down)) {
            chosenControl = kPlayer1Down;
            chosenControlIndex = kPlayer1DownControl;
          }
          if (menu_active_is (controlsMenu, kPlayer1Clockwise)) {
            chosenControl = kPlayer1Clockwise;
            chosenControlIndex = kPlayer1ClockwiseControl;
          }
          if (menu_active_is (controlsMenu, kPlayer1Counterclockwise)) {
            chosenControl = kPlayer1Counterclockwise;
            chosenControlIndex = kPlayer1CounterclockwiseControl;
          }

          if (menu_active_is (controlsMenu, kPlayer2Left)) {
            chosenControl = kPlayer2Left;
            chosenControlIndex = kPlayer2LeftControl;
          }
          if (menu_active_is (controlsMenu, kPlayer2Right)) {
            chosenControl = kPlayer2Right;
            chosenControlIndex = kPlayer2RightControl;
          }
          if (menu_active_is (controlsMenu, kPlayer2Down)) {
            chosenControl = kPlayer2Down;
            chosenControlIndex = kPlayer2DownControl;
          }
          if (menu_active_is (controlsMenu, kPlayer2Clockwise)) {
            chosenControl = kPlayer2Clockwise;
            chosenControlIndex = kPlayer2ClockwiseControl;
          }
          if (menu_active_is (controlsMenu, kPlayer2Counterclockwise)) {
            chosenControl = kPlayer2Counterclockwise;
            chosenControlIndex = kPlayer2CounterclockwiseControl;
          }

          if (chosenControl != NULL) {
            char prevValue[255];
            strcpy(prevValue, menu_get_value (controlsMenu, chosenControl));
            menu_set_value(controlsMenu, chosenControl, "<Choose>");
            if (changeControlLoop(chosenControlIndex, d)) {
              getKeyName(chosenControlIndex, newKeyName);
              menu_set_value(controlsMenu, chosenControl, newKeyName);
              menu_next_item (controlsMenu);
            }
            else {
              menu_set_value(controlsMenu, chosenControl, prevValue);
            }
          }
          break;
        case GameControlEvent::kBack:
          goto mml_fin;
          break;
        default:
          break;
      }
    }

    updateAll(d);
  }
mml_fin:
  saveControls();
  menu_hide (controlsMenu);
}

void PuyoCommander::optionMenuLoop(PuyoDrawable *d)
{
  menu_show(optionMenu);
  while (1) {
    SDL_Event e;

    while (SDL_PollEvent (&e)) {
      GameControlEvent controlEvent;
      getControlEvent(e, &controlEvent);

      switch (controlEvent.cursorEvent) {
        case GameControlEvent::kQuit:
          exit(0);
          goto mml_fin;
          break;
        case GameControlEvent::kUp:
          menu_prev_item (optionMenu);
          break;
        case GameControlEvent::kDown:
          menu_next_item (optionMenu);
          break;
        case GameControlEvent::kStart:
          menu_validate (optionMenu);
          if (menu_active_is (optionMenu, "Back"))
            goto mml_fin;
          if (menu_active_is (optionMenu, kFullScreen)) {
            fullscreen  = menu_switch_on_off(optionMenu, kFullScreen);
            SetBoolPreference(kFullScreen,fullscreen);
            if (useGL)
            {
              menu_hide(optionMenu);
              backLoop(mustRestartMenu,d);
              menu_show(optionMenu);
            }
            else
            {
#ifdef _WIN32
                menu_hide(optionMenu);
                backLoop(mustRestartMenu,d);
                menu_show(optionMenu);
#else
                SDL_QuitSubSystem(SDL_INIT_VIDEO);
                SDL_InitSubSystem(SDL_INIT_VIDEO);
                display = SDL_SetVideoMode(640, 480, 0,
                                           SDL_ANYFORMAT|SDL_HWSURFACE|SDL_DOUBLEBUF
                                           |(fullscreen?SDL_FULLSCREEN:0)|(useGL?SDL_GLSDL:0));
#endif
              /* IIM_ReConvertAll();
              SoFont_Refresh(menuFont);
              SoFont_Refresh(smallFont);
              SoFont_Refresh(darkFont); */
            }
          }
          if (menu_active_is (optionMenu, kOpenGL)) {
            bool useGL2 = menu_switch_on_off(optionMenu, kOpenGL);
            SetBoolPreference(kOpenGL,useGL2);
            menu_hide(optionMenu);
            backLoop(mustRestartMenu,d);
            menu_show(optionMenu);
          }
          if (menu_active_is (optionMenu, kMusic)) {
            sound = menu_switch_on_off(optionMenu, kMusic);
            SetBoolPreference(kMusic,sound);
            audio_set_music_on_off(sound);
            if (sound) audio_music_start(0);
          }
          if (menu_active_is (optionMenu, kAudioFX)) {
            fx = menu_switch_on_off(optionMenu, kAudioFX);
            SetBoolPreference(kAudioFX,fx);
            audio_set_sound_on_off(fx);
          }
          if (menu_active_is (optionMenu, kControls)) {
            menu_hide (optionMenu);
            controlsMenuLoop(d);
            menu_show(optionMenu);
          }
          break;
        case GameControlEvent::kBack:
          goto mml_fin;
          break;
        default:
          break;
      }
    }

    updateAll(d);
  }
mml_fin:
  menu_hide (optionMenu);
}

void PuyoCommander::backLoop(Menu *menu, PuyoDrawable *starter)
{

    if (menu == aboutMenu) {
        corona = corona32_new();
        corona32_resize(corona, 640, CORONA_HEIGHT);
        corona_screen = (int*)calloc(640 * CORONA_HEIGHT + 64, sizeof(int));
    }

    menu_show(menu);
    while (1) {
        SDL_Event e;
        
        while (SDL_PollEvent (&e)) {
            GameControlEvent controlEvent;
            getControlEvent(e, &controlEvent);
            
            switch (controlEvent.cursorEvent) {
                case GameControlEvent::kQuit:
                    exit(0);
                    goto mml_fin;
                case GameControlEvent::kStart:
                case GameControlEvent::kBack:
                    goto mml_fin;
                    break;
            }
        }
        updateAll(starter);
    }
mml_fin:
    menu_hide (menu);
    if (corona)
    {
        corona32_delete(corona);
        free(corona_screen);
        corona = NULL;
    }
}

void PuyoCommander::enterStringLoop(Menu *menu, const char *kItem, char out[256])
{
  int len = 0;
  char prevValue[256];
  strcpy(prevValue, menu_get_value (menu, kItem));
  out[0] = '_';
  out[1] = 0;
  menu_set_value(menu, kItem, "_");

  while (1)
  {
    SDL_Event e;
    
    while (SDL_PollEvent (&e)) {
      GameControlEvent controlEvent;
      getControlEvent(e, &controlEvent);
      switch (controlEvent.cursorEvent) {
        case GameControlEvent::kQuit:
          exit(0);
          break;
        case GameControlEvent::kStart:
          out[len] = 0;
          menu_set_value(menu, kItem, out);
          menu_validate (menu);
          return;
        case GameControlEvent::kBack:
          strcpy(out, prevValue);
          menu_set_value(menu, kItem, prevValue);
          return;
      }
      switch (e.type) {
        case SDL_KEYDOWN:
          {
            char ch = 0;
            if ((e.key.keysym.sym >= SDLK_a) && (e.key.keysym.sym <= SDLK_z))
              ch = e.key.keysym.sym;

            if ((ch >= 'a') && (ch <= 'z')) {
              if ((len == 0) || (out[len-1] == ' '))
                ch = ch - 'a' + 'A';
            }

            if (e.key.keysym.sym == SDLK_SPACE)
              ch = ' ';

            if ((ch!=0) && (len < 10)) {
              out[len++] = ch;
              out[len]   = '_';
              out[len+1] = 0;
            }

            if ((e.key.keysym.sym == SDLK_BACKSPACE) && (len > 0))
            {
              out[len] = 0;
              out[--len] = '_';
            }
            menu_set_value(menu, kItem, out, 0);
          }
          break;
      }
    }
    updateAll(NULL);
  }
}


void PuyoCommander::startTwoPlayerGameLoop()
{
  char player1Name[256];
  char player2Name[256];

  GetStrPreference("Player1 Name", player1Name, "Player 1");
  GetStrPreference("Player2 Name", player2Name, "Player 2");
  menu_set_value(twoPlayerGameMenu, kPlayer1Name, player1Name, 0);
  menu_set_value(twoPlayerGameMenu, kPlayer2Name, player2Name, 0);

  while (!menu_active_is(twoPlayerGameMenu,kLevelMedium))
    menu_next_item(twoPlayerGameMenu);  

  menu_show(twoPlayerGameMenu);
  while (1) {
    SDL_Event e;

    while (SDL_PollEvent (&e)) {
      GameControlEvent controlEvent;
        getControlEvent(e, &controlEvent);
        switch (controlEvent.cursorEvent) {
            case GameControlEvent::kQuit:
                exit(0);
                break;
            case GameControlEvent::kDown:
                menu_next_item (twoPlayerGameMenu);
                break;
            case GameControlEvent::kUp:
                menu_prev_item (twoPlayerGameMenu);
                break;
            case GameControlEvent::kStart:
                menu_validate (twoPlayerGameMenu);
                if (menu_active_is(twoPlayerGameMenu,kPlayer2Name)) {
                  enterStringLoop(twoPlayerGameMenu,kPlayer2Name,player2Name);
                  menu_next_item (twoPlayerGameMenu);
                }
                else if (menu_active_is(twoPlayerGameMenu,kPlayer1Name)) {
                  enterStringLoop(twoPlayerGameMenu,kPlayer1Name,player1Name);
                  menu_next_item (twoPlayerGameMenu);
                }
                else
                  goto mml_play;
                break;
            case GameControlEvent::kBack:
                menu_hide(twoPlayerGameMenu);
                return;
                break;
        }
    }
    updateAll(NULL);
  }
  
mml_play:
  menu_hide (twoPlayerGameMenu);

  GAME_ACCEL = 2000;
  gameLevel = 1;
  if (menu_active_is (twoPlayerGameMenu, kLevelMedium)) {
    GAME_ACCEL = 1500;
    gameLevel = 2;
  }
  else if (menu_active_is (twoPlayerGameMenu, kLevelHard)) {
    GAME_ACCEL = 1000;
    gameLevel = 3;
  }

  int score1 = 0;
  int score2 = 0;
  gameOverMenu = gameOver2PMenu;
  int currentMusicTheme = 0;
  if (menu_active_is(gameOverMenu, "NO"))
    menu_next_item(gameOverMenu);
  while (menu_active_is(gameOverMenu, "YES")) {
    menu_next_item(gameOverMenu);
    PuyoStarter myStarter(this,false,0,RANDOM,currentMusicTheme);
    audio_music_switch_theme(currentMusicTheme);
    p1name = player1Name;
    p2name = player2Name;
    GAME_ACCEL = 1500;
    doom_melt_start(melt, menuBGImage);
    myStarter.run(score1, score2, 0, 0, 0);
    score1 += myStarter.leftPlayerWin();
    score2 += myStarter.rightPlayerWin();
    currentMusicTheme = (currentMusicTheme + 1) % NB_MUSIC_THEME;
  }

  SetStrPreference("Player1 Name", player1Name);
  SetStrPreference("Player2 Name", player2Name);
  doom_melt_start(melt, gameScreen);
}


struct SelIA {
  IA_Type type;
  int level;
};

void PuyoCommander::startSingleGameLoop()
{
  char playerName[256];

    char * defaultName = getenv("USER");
    if (defaultName == NULL)
      defaultName = "Player";
    if (!(defaultName[0]>=32))
      defaultName = "Player";
    if ((defaultName[0]>='a') && (defaultName[0]<='z'))
      defaultName[0] += 'A' - 'a';
    
  GetStrPreference("Player Name", playerName, defaultName);
  menu_set_value(singleGameMenu, kPlayerName, playerName, 0);

  while (!menu_active_is(singleGameMenu,kLevelMedium))
    menu_next_item(singleGameMenu);  
  
  menu_show(singleGameMenu);
  while (1) {
    SDL_Event e;

    while (SDL_PollEvent (&e)) {
      GameControlEvent controlEvent;
        getControlEvent(e, &controlEvent);
        switch (controlEvent.cursorEvent) {
            case GameControlEvent::kQuit:
                exit(0);
                break;
            case GameControlEvent::kDown:
                menu_next_item (singleGameMenu);
                break;
            case GameControlEvent::kUp:
                menu_prev_item (singleGameMenu);
                break;
            case GameControlEvent::kStart:
                menu_validate (singleGameMenu);
                if (menu_active_is(singleGameMenu,kPlayerName))
                  enterStringLoop(singleGameMenu,kPlayerName,playerName);
                else
                  goto mml_play;
                break;
            case GameControlEvent::kBack:
                menu_hide(singleGameMenu);
                return;
                break;
        }
    }
    updateAll(NULL);
  }
  
mml_play:
  menu_hide (singleGameMenu);

  SelIA ia1[] = { {RANDOM, 350}, {FLOBO, 350}, {FLOBO, 250}, {FLOBO, 180}, {FLOBO,  90}, {JEKO, 350}, {TANIA, 320}, {FLOBO, 62}, {RANDOM,0} };
  SelIA ia2[] = { {FLOBO,  190}, {JEKO , 180}, {TANIA, 160}, {FLOBO,  90}, {GYOM , 210}, {TANIA, 90}, {JEKO,   80}, {GYOM,  90}, {RANDOM,0} };
  SelIA ia3[] = { {TANIA,  130}, {JEKO , 100}, {GYOM ,  90}, {JEKO,   80}, {TANIA,  60}, {GYOM,  60}, {GYOM,   40}, {GYOM,  30}, {RANDOM,0} };

  SelIA *ia = &(ia1[0]);

  gameLevel = 1;
  GAME_ACCEL = 2000;
  if (menu_active_is (singleGameMenu, kLevelMedium)) {
    ia = &(ia2[0]);
    GAME_ACCEL = 1500;
    gameLevel = 2;
  }
  else if (menu_active_is (singleGameMenu, kLevelHard)) {
    ia = &(ia3[0]);
    GAME_ACCEL = 1000;
    gameLevel = 3;
  }
  
  int score1 = 0;
  int score2 = 0;
  int lives  = 3;
  if (menu_active_is(finishedMenu,  "NO"))
    menu_next_item(finishedMenu);
  if (menu_active_is(looserMenu   ,  "NO"))
    menu_next_item(looserMenu);
  if (menu_active_is(nextLevelMenu,  "NO"))
    menu_next_item(nextLevelMenu);
  if (menu_active_is(gameOver2PMenu, "NO"))
    menu_next_item(gameOver2PMenu);
  gameOverMenu = nextLevelMenu;

  int lastPoints = 0;
  int currentMusicTheme = 0;
  
  int fini = 0;  
  while (!fini)
  {
    PuyoStory myStory(this, score2+1);
    myStory.loop();
    PuyoStarter myStarter(this, true, ia[score2].level, ia[score2].type, currentMusicTheme);
    p1name = playerName;
    p2name = AI_NAMES[score2];
    doom_melt_start(melt, menuBGImage);
    audio_music_switch_theme(currentMusicTheme);
    myStarter.run(score1, score2, lives, lastPoints, 0);
    lastPoints = myStarter.rightPlayerPoints();
    score1 += myStarter.leftPlayerWin();
    score2 += myStarter.rightPlayerWin();
    if (!myStarter.rightPlayerWin())
      lives--;
    else {
      currentMusicTheme = (currentMusicTheme + 1) % NB_MUSIC_THEME;
    }

    if (ia[score2].level == 0) {
      lastPoints += 100000;
      if (menu_active_is (singleGameMenu, kLevelMedium))
        lastPoints += 150000;
      if (menu_active_is (singleGameMenu, kLevelHard))
        lastPoints += 250000;
      fini = 1;
    }
  
    if (!(menu_active_is(gameOverMenu, "YES") && (lives >= 0)))
      fini = 1;
  
    if (fini)
    {
      audio_music_start(0);
      int newOne = setHiScore(lastPoints, p1name);
      if (newOne >= 0)
      {
        updateHighScoresMenu(newOne);
        backLoop(highScoresMenu, &myStarter);
      }
    }
  }
  SetStrPreference("Player Name", playerName);
  doom_melt_start(melt, gameScreen);
}

#define TIME_TOLERANCE 4

void PuyoCommander::updateAll(PuyoDrawable *starter, SDL_Surface *extra_surf)
{
  Uint32  now = 0;

  // mise a jour
  menu_update (mainMenu, display);

  menu_update(gameOver2PMenu, display);
  menu_update(gameOver1PMenu, display);
  menu_update(nextLevelMenu, display);
  menu_update(finishedMenu, display);
  menu_update(looserMenu, display);
  
  menu_update (optionMenu, display);
  menu_update (controlsMenu, display);
  menu_update (rulesMenu, display);
  menu_update (highScoresMenu, display);
  menu_update (aboutMenu, display);
  menu_update (mustRestartMenu, display);
  menu_update (singleGameMenu, display);
  menu_update (twoPlayerGameMenu, display);
  menu_update (menu_pause,display);
  scrolling_text_update(scrollingText, display);
  doom_melt_update(melt);
  
  // affichage eventuel (pourrait ne pas avoir lieu de tps en tps si machine
  // trop lente)
  cycle++;
  now = SDL_GetTicks ();

  if ((now < (start_time + (cycle + TIME_TOLERANCE) * cycle_duration))
                  || (cycle - lastRenderedCycle > maxFrameDrop))
  {
    lastRenderedCycle = cycle;
    
    if (starter) {
      starter->draw();
    }
    else {
      SDL_BlitSurface (menuBGImage->surf, NULL, display, NULL);
    }

    if (corona)
    {
        short frequency[2][512];
        for (int i=0; i<512; ++i) { // Generate random sound.
            frequency[0][i] = rand();
            frequency[1][i] = rand();
        }
        corona32_update(corona, SDL_GetTicks(), frequency);
        corona32_displayRGBA(corona, corona_screen);
        SDL_Surface *tmpsurf =
            SDL_CreateRGBSurfaceFrom (corona_screen, 640, CORONA_HEIGHT,
                                      32, 640*4,
                                      0x00ff0000, 0x0000ff00, 0x000000ff,
                                      0xff000000);
        SDL_Rect rect;
        rect.x = 0;
        rect.y = 480 - tmpsurf->h;
        rect.w = tmpsurf->w;
        rect.h = tmpsurf->h;
        SDL_BlitSurface(tmpsurf, NULL, display, &rect);
        SDL_FreeSurface (tmpsurf);
    }
    if (extra_surf)
    {
        SDL_Rect rect;
        rect.x = 0;
        rect.y = 480 - extra_surf->h;
        rect.w = extra_surf->w;
        rect.h = extra_surf->h;
        SDL_BlitSurface(extra_surf, NULL, display, &rect);
    }

    if (!starter)
      scrolling_text_draw(scrollingText, display, 460);

    menu_draw (mainMenu, display);
    menu_draw(gameOver2PMenu, display);
    menu_draw(gameOver1PMenu, display);
    menu_draw(nextLevelMenu, display);
    menu_draw(finishedMenu, display);
    menu_draw(looserMenu, display);
    menu_draw (optionMenu, display);
    menu_draw (controlsMenu, display);
    menu_draw (rulesMenu, display);
    menu_draw (highScoresMenu, display);
    menu_draw (aboutMenu, display);
    menu_draw (mustRestartMenu, display);
    menu_draw (singleGameMenu, display);
    menu_draw (twoPlayerGameMenu, display);
    menu_draw(menu_pause,display);
    doom_melt_display(melt, display);
    SDL_Flip (display);
  }

  // delay si machine trop rapide
  now = SDL_GetTicks ();
  if (now < (start_time + cycle * cycle_duration)) {
    int ttw = ((start_time + cycle * cycle_duration) - now);
    ttw /= 2;
    SDL_Delay (ttw);
  }
}


