#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "PuyoCommander.h"
#include "PuyoView.h"
#include "PuyoVersion.h"
#include "PuyoStory.h"
#include "preferences.h"
#include "InputManager.h"

#ifndef DATADIR
extern char *DATADIR;
#endif

char *kYouDidIt = "You Dit It!!!";
char *kNextLevel = "Next Level:";
char *kLooser = "Looser!!!";
char *kCurrentLevel = "Current Level:";
char *kContinueLeft = "Continue Left:";
char *kGameOver = "Game Over!!!";
char *kYouGotToLevel = "You get to level";
char *kHitActionToContinue = "Hit Action to continue...";
char *kHitActionForMenu = "Hit Action for menu...";
char *kContinue    = "Continue?";
char *kContinueGame= "Continue Game";
char *kAbortGame   = "Abort Game";
char *kOptions     = "Options";
char *kPlayer      = "Player";
char *kScore       = "Score:";

static char *kMusicVolume = "MusicVolume";
static char *kAudioVolume = "AudioVolume";

char *kCongratulations = "Congratulations!!!";
char *kPuyosInvasion   = "You stopped Puyo's invasion.\n"
                         "Peace on Earth is restored!!";

static char *kAudioFX     = "Audio FX";
static char *kMusic       = "Music";
static char *kFullScreen  = "FullScreen";
static char *kControls    = "Change controls...";
static char *kGameLevel   = "Choose Game Level";
static char *kLevelEasy   = "Easy";
static char *kLevelMedium = "Medium";
static char *kLevelHard   = "Hard";

static char *kPlayer1Left  = "P1  Left:";
static char *kPlayer1Right = "P1 Right:";
static char *kPlayer1Down  = "P1  Down:";
static char *kPlayer1Clockwise        = "P1 Turn L:";
static char *kPlayer1Counterclockwise = "P1 Turn R:";

static char *kPlayer2Left   = "P2  Left:";
static char *kPlayer2Right  = "P2 Right:";
static char *kPlayer2Down   = "P2  Down:";
static char *kPlayer2Clockwise        = "P2 Turn L:";
static char *kPlayer2Counterclockwise = "P2 Turn R:";

static char *kRules01 = "Puyos are fancy smiling bubbles...";
static char *kRules02 = "But they can really be invading sometimes!";
static char *kRules03 = "Send them away by making groups of 4 or more.";
static char *kRules04 = "Try to make large groups, or many at the same time";
static char *kRules05 = "to send more bad ghost Puyos to your opponent.";

static char *kAbout01 = "FloboPuyo is an iOS-software production.";
static char *kAbout02 = "Puyo's world Meta God :          ";
static char *kAbout03 = "                     Florent 'flobo' Boudet";
static char *kAbout04 = "X Men:                                    ";
static char *kAbout05 = "          Jean-Christophe 'jeko' Hoelt";
static char *kAbout06 = "                   Guillaume 'gyom' Borios";
static char *kAbout07 = "Beta Goddess:                        ";
static char *kAbout08 = "                                           Tania";

extern SDL_Surface *display, *image;
static SDL_Surface *menuBGImage = 0;
PuyoCommander *theCommander;
SoFont *smallFont,*menuFont,*darkFont;

const int cycle_duration = 20;

static bool fullscreen = true;
static bool sound = true;
static bool fx = true;

SDL_Surface * IMG_Load_DisplayFormat (const char *fname)
{
  char path[1024];
  SDL_Surface *tmpsurf, *retsurf;
  sprintf(path, "%s/gfx/%s", DATADIR, fname);

  tmpsurf = IMG_Load (path);
  if (tmpsurf==0) {
    fprintf(stderr,"Could not load %s\n", path);
    exit(1);
  }
  retsurf = SDL_DisplayFormat (tmpsurf);
  SDL_FreeSurface (tmpsurf);
  return retsurf;
}

SDL_Surface * IMG_Load_DisplayFormatAlpha (const char *fname)
{
  char path[1024];
  SDL_Surface *tmpsurf, *retsurf;
  sprintf(path, "%s/gfx/%s", DATADIR, fname);

  tmpsurf = IMG_Load (path);
  if (tmpsurf==0) {
    fprintf(stderr,"Could not load %s\n", path);
    exit(1);
  }
  retsurf = SDL_DisplayFormatAlpha (tmpsurf);
  SDL_SetAlpha (retsurf, SDL_SRCALPHA | SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
  SDL_FreeSurface (tmpsurf);
  return retsurf;
}

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
    menu_items_set_font_for (main_menu, "Options", font);
    menu_items_set_font_for (main_menu, "Rules", font);
    menu_items_set_font_for (main_menu, "About FloboPuyo", font);
    menu_items_set_font_for (main_menu, "Quit", font);
    loaded = 1;
    menuBGImage = IMG_Load_DisplayFormat("MenuBackground.jpg");
  }

  return main_menu;
}


MenuItems single_game_menu_load (SoFont *font, SoFont *small_font)
{
  static MenuItemsTab go_menu =
  {
    MENUITEM_INACTIVE(kGameLevel),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM(kLevelEasy),
    MENUITEM_BLANKLINE,
    MENUITEM(kLevelMedium),
    MENUITEM_BLANKLINE,
    MENUITEM(kLevelHard),
    MENUITEM_END
  };
  menu_items_set_font_for(go_menu, kGameLevel,   font);
  menu_items_set_font_for(go_menu, kLevelEasy,   font);
  menu_items_set_font_for(go_menu, kLevelMedium, font);
  menu_items_set_font_for(go_menu, kLevelHard,   font);
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

MenuItems options_menu_load (SoFont *font, SoFont *small_font)
{
  static MenuItemsTab option_menu = {
#ifndef _WIN32
    MENUITEM(kFullScreen),
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
#ifndef _WIN32
  menu_items_set_font_for(option_menu,  kFullScreen, font);
  menu_items_set_value_for(option_menu, kFullScreen, fullscreen?"ON":"OFF");
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
    MENUITEM(kPlayer1Left),
    MENUITEM(kPlayer1Right),
    MENUITEM(kPlayer1Down),
    MENUITEM(kPlayer1Clockwise),
    MENUITEM(kPlayer1Counterclockwise),
    MENUITEM_BLANKLINE,
    MENUITEM(kPlayer2Left),
    MENUITEM(kPlayer2Right),
    MENUITEM(kPlayer2Down),
    MENUITEM(kPlayer2Clockwise),
    MENUITEM(kPlayer2Counterclockwise),
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
  fullscreen = GetBoolPreference(kFullScreen,fs);
  sound = GetBoolPreference(kMusic,snd);
  fx = GetBoolPreference(kAudioFX,audio);

  int music_volume = GetIntPreference(kMusicVolume, 100);
  int audio_volume = GetIntPreference(kAudioVolume, 80);

  initGameControls();
   
#ifdef __linux__
  /* This Hack Allows Hardware Surface on Linux */
  setenv("SDL_VIDEODRIVER","dga",0);

  if (SDL_Init(init_flags) < 0) {
    printf(":-( Could not use DGA.\n");
    setenv("SDL_VIDEODRIVER","x11",1);
    if (SDL_Init(init_flags) < 0) {
      fprintf(stderr, "SDL initialisation error:  %s\n", SDL_GetError());
      exit(1);
    }
  }
  else {
    fullscreen = fs = true;
    printf(":-) DGA Available !\n");
    SDL_WM_GrabInput(SDL_GRAB_ON);
  }
#else
  if ( SDL_Init(init_flags) < 0 ) {
    fprintf(stderr, "SDL initialisation error:  %s\n", SDL_GetError());
    exit(1);
  }
#endif

  initControllers();
  
  audio_init();
  audio_music_start(0);
  if (sound==false) Mix_PauseMusic();
  audio_set_music_on_off(sound);
  audio_set_sound_on_off(fx);

  audio_set_volume(audio_volume);
  audio_music_set_volume(music_volume);

  display = SDL_SetVideoMode( 640, 480, 0,  SDL_ANYFORMAT|SDL_HWSURFACE|SDL_DOUBLEBUF|(fullscreen?SDL_FULLSCREEN:0));
  if ( display == NULL ) {
    fprintf(stderr, "SDL_SetVideoMode error: %s\n",
            SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);
  SDL_ShowCursor(SDL_DISABLE);

  smallFont = SoFont_new();
  SoFont_load (smallFont, IMG_Load_DisplayFormatAlpha ("font4b.png"));
  menuFont = SoFont_new();
  SoFont_load (menuFont, IMG_Load_DisplayFormatAlpha ("font3b.png"));
  darkFont = SoFont_new();
  SoFont_load (darkFont, IMG_Load_DisplayFormatAlpha ("fontdark.png"));

  SDL_Surface * menuselector = IMG_Load_DisplayFormatAlpha("menusel.png");

  mainMenu = menu_new(main_menu_load(menuFont),menuselector);
  gameOver1PMenu = menu_new(gameover_1p_menu_load(menuFont, smallFont),menuselector);
  gameOver2PMenu = menu_new(gameover_2p_menu_load(menuFont, smallFont),menuselector);
  nextLevelMenu  = menu_new(nextlevel_1p_menu_load(menuFont, smallFont),menuselector);
  looserMenu     = menu_new(looser_1p_menu_load(menuFont, smallFont),menuselector);
  finishedMenu   = menu_new(finished_1p_menu_load(menuFont, smallFont),menuselector);
  gameOverMenu   = gameOver2PMenu;
  optionMenu = menu_new(options_menu_load(menuFont, smallFont),menuselector);
  controlsMenu = menu_new(controls_menu_load(menuFont, smallFont),menuselector);
  rulesMenu = menu_new(rules_menu_load(menuFont),menuselector);
  aboutMenu = menu_new(about_menu_load(menuFont),menuselector);
  singleGameMenu = menu_new(single_game_menu_load(menuFont,smallFont),menuselector);
  if (menu_pause == NULL) menu_pause = menu_new(pause_menu_load(menuFont),menuselector);
  menu_set_sounds (optionMenu,     sound_pop, sound_slide);
  menu_set_sounds (controlsMenu,   sound_pop, sound_slide);
  menu_set_sounds (mainMenu,       sound_pop, sound_slide);
  menu_set_sounds (rulesMenu,      sound_pop, sound_slide);
  menu_set_sounds (aboutMenu,      sound_pop, sound_slide);
  menu_set_sounds (singleGameMenu, sound_pop, sound_slide);
  menu_set_sounds (menu_pause    , sound_pop, sound_slide);

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
                        int score1 = 0;
                        int score2 = 0;
                        menu_hide (menu);
                        gameOverMenu = gameOver2PMenu;
                        int currentMusicTheme = 0;
                        if (menu_active_is(gameOverMenu, "NO"))
                            menu_next_item(gameOverMenu);
                        while (menu_active_is(gameOverMenu, "YES")) {
                            menu_next_item(gameOverMenu);
                            PuyoStarter myStarter(this,false,0,RANDOM,currentMusicTheme);
                            audio_music_switch_theme(currentMusicTheme);
                            myStarter.run(score1, score2, 0);
                            score1 += myStarter.leftPlayerWin();
                            score2 += myStarter.rightPlayerWin();
                            currentMusicTheme = (currentMusicTheme + 1) % NB_MUSIC_THEME;
                        }
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
                        display = SDL_SetVideoMode( 640, 480, 0,  SDL_ANYFORMAT|SDL_HWSURFACE|SDL_DOUBLEBUF|(fullscreen?SDL_FULLSCREEN:0));
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

void PuyoCommander::backLoop(Menu *menu)
{
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
        
        updateAll(NULL);
    }
mml_fin:
        menu_hide (menu);
}

struct SelIA {
  IA_Type type;
  int level;
};

void PuyoCommander::startSingleGameLoop()
{
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

  SelIA ia1[] = { {RANDOM, 250}, {FLOBO, 250}, {FLOBO, 150}, {FLOBO, 100}, {FLOBO,  60}, {JEKO, 250}, {TANIA, 220}, {FLOBO, 32}, {RANDOM,0} };
  SelIA ia2[] = { {FLOBO,  120}, {JEKO , 110}, {TANIA, 100}, {FLOBO,  50}, {GYOM , 120}, {TANIA, 60}, {JEKO,   50}, {GYOM,  60}, {RANDOM,0} };
  SelIA ia3[] = { {TANIA,  100}, {JEKO ,  80}, {GYOM ,  80}, {JEKO,   50}, {TANIA,  30}, {GYOM,  30}, {GYOM,   20}, {GYOM,  10}, {RANDOM,0} };

  SelIA *ia = &(ia1[0]);

  if (menu_active_is (singleGameMenu, kLevelMedium))
    ia = &(ia2[0]);
  else if (menu_active_is (singleGameMenu, kLevelHard))
    ia = &(ia3[0]);
  
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

  int currentMusicTheme = 0;
  audio_music_switch_theme(currentMusicTheme);
  
  while (menu_active_is(gameOverMenu, "YES") && (lives >= 0)) {
    
    PuyoStory myStory(this, score2+1);
    myStory.loop();
    PuyoStarter myStarter(this, true, ia[score2].level, ia[score2].type, currentMusicTheme);
    myStarter.run(score1, score2, lives);
    score1 += myStarter.leftPlayerWin();
    score2 += myStarter.rightPlayerWin();
    if (!myStarter.rightPlayerWin())
      lives--;
    else {
      currentMusicTheme = (currentMusicTheme + 1) % NB_MUSIC_THEME;
      audio_music_switch_theme(currentMusicTheme);
    }
    
    if (ia[score2].level == 0) {
      break;
    }
  }
  audio_music_start(0);
}

void PuyoCommander::updateAll(PuyoDrawable *starter)
{
  Uint32  now = 0;

  // mise a jour
  menu_update (mainMenu, display);
  menu_update (gameOverMenu, display);
  menu_update (optionMenu, display);
  menu_update (controlsMenu, display);
  menu_update (rulesMenu, display);
  menu_update (aboutMenu, display);
  menu_update (singleGameMenu, display);
  menu_update (menu_pause,display);
  scrolling_text_update(scrollingText, display);

  // affichage eventuel (pourrait ne pas avoir lieu de tps en tps si machine
  // trop lente)
  cycle++;
  now = SDL_GetTicks ();

  if (now < (start_time + cycle * cycle_duration)) {

    if (starter) {
      starter->draw();
    }
    else {
      SDL_BlitSurface (menuBGImage, NULL, display, NULL);
      scrolling_text_draw(scrollingText, display, 460);
    }

    menu_draw (mainMenu, display);
    menu_draw (gameOverMenu, display);
    menu_draw (optionMenu, display);
    menu_draw (controlsMenu, display);
    menu_draw (rulesMenu, display);
    menu_draw (aboutMenu, display);
    menu_draw (singleGameMenu, display);
    menu_draw(menu_pause,display);
    SDL_Flip (display);
  }

  // delay si machine trop rapide
  now = SDL_GetTicks ();
  if (now < (start_time + cycle * cycle_duration))
    SDL_Delay ((start_time + cycle * cycle_duration) - now);
}


