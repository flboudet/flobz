#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "PuyoCommander.h"
#include "PuyoView.h"
#include "PuyoStory.h"
#include "preferences.h"

#ifndef DATADIR
extern char *DATADIR;
#endif

static char *kAudioFX     = "Audio FX";
static char *kMusic       = "Music";
static char *kFullScreen  = "FullScreen";
static char *kControls    = "Change controls...";
static char *kGameLevel   = "Choose Game Level";
static char *kLevelEasy   = "Easy";
static char *kLevelMedium = "Medium";
static char *kLevelHard   = "Hard";

static char *kRules01 = "A puyo is a small colored 'smiley'.. Your goal:";
static char *kRules02 = "Create blobs of 4 or more puyos having the same color.";
static char *kRules03 = "Each turn you are given 2 new puyos to drop.";
static char *kRules04 = "Try to make large blobs, or many at the same time";
static char *kRules05 = "to give some bad grey puyos to your oponent.";

static char *kRules10 = "Left player uses s,d,f,e to move.";
static char *kRules11 = "Right player uses the arrows to move.";

static char *kAbout01 = "FloboPuyo is an iOS-software production.";
static char *kAbout02 = "Main work:                              ";
static char *kAbout03 = "                 Florent 'flobo' Boudet ";
static char *kAbout04 = "Menus / Small Patches:";
static char *kAbout05 = "            Jean-Christophe 'jeko' Hoelt";
static char *kAbout06 = "Beta Testing:";
static char *kAbout07 = "                   the iOS-software team";
static char *kAbout08 = "                        (and friends...)";

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

MenuItems gameover_menu_load (SoFont *font, SoFont *small_font)
{
  static MenuItemsTab go_menu = {
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE("Game Over!!!"),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE("Winner: "),
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE("Current"),
    MENUITEM_BLANKLINE,
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE("Continue? (y/n)"),
    MENUITEM("YES"),
    MENUITEM("NO"),
    MENUITEM_END
  };
  menu_items_set_font_for(go_menu, "Game Over!!!",    font);
  menu_items_set_font_for(go_menu, "Winner: ",  small_font);
  menu_items_set_font_for(go_menu, "Current",   small_font);
  menu_items_set_font_for(go_menu, "Continue? (y/n)", font);
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
    MENUITEM_INACTIVE(kRules10),
    MENUITEM_INACTIVE(kRules11),
    MENUITEM_BLANKLINE,
    MENUITEM("Back"),
    MENUITEM_END
  };
  menu_items_set_font_for(option_menu, kRules01, font);
  menu_items_set_font_for(option_menu, kRules02, font);
  menu_items_set_font_for(option_menu, kRules03, font);
  menu_items_set_font_for(option_menu, kRules04, font);
  menu_items_set_font_for(option_menu, kRules05, font);
  menu_items_set_font_for(option_menu, kRules10, font);
  menu_items_set_font_for(option_menu, kRules11, font);

  return option_menu;
}

MenuItems about_menu_load (SoFont *font)
{
  static MenuItemsTab option_menu = {
    MENUITEM_INACTIVE(kAbout01),
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kAbout02),
    MENUITEM_INACTIVE(kAbout03),
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kAbout04),
    MENUITEM_INACTIVE(kAbout05),
    MENUITEM_BLANKLINE,
    MENUITEM_INACTIVE(kAbout06),
    MENUITEM_INACTIVE(kAbout07),
    MENUITEM_INACTIVE(kAbout08),
    MENUITEM_BLANKLINE,
    MENUITEM("Back"),
    MENUITEM_END
  };
  menu_items_set_font_for(option_menu, kAbout01, font);
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

PuyoCommander::PuyoCommander(bool fs, bool snd, bool audio)
{
  int init_flags = SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK;

  SDL_Delay(500);
  fullscreen = GetBoolPreference(kFullScreen,fs);
  sound = GetBoolPreference(kMusic,snd);
  fx = GetBoolPreference(kAudioFX,audio);

#ifdef __linux__
  /* This Hack Allows Hardware Surface on Linux */
  setenv("SDL_VIDEODRIVER","dga",0);

  if (SDL_Init(init_flags) < 0) {
    printf(":-( Could not use DGA. Try using FloboPuyo as root.\n");
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
  audio_set_music_on_off(sound);
  audio_set_sound_on_off(fx);

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
  gameOverMenu = menu_new(gameover_menu_load(menuFont, smallFont),menuselector);
  optionMenu = menu_new(options_menu_load(menuFont, smallFont),menuselector);
  rulesMenu = menu_new(rules_menu_load(menuFont),menuselector);
  aboutMenu = menu_new(about_menu_load(menuFont),menuselector);
  singleGameMenu = menu_new(single_game_menu_load(menuFont,smallFont),menuselector);
  menu_set_sounds (optionMenu,     sound_pop, sound_slide);
  menu_set_sounds (mainMenu,       sound_pop, sound_slide);
  menu_set_sounds (rulesMenu,      sound_pop, sound_slide);
  menu_set_sounds (aboutMenu,      sound_pop, sound_slide);
  menu_set_sounds (singleGameMenu, sound_pop, sound_slide);

  scrollingText = scrolling_text_new(
    "Welcome to the wonderful world of FloboPuyo !!! Enjoy its nice graphics, "
    "happy music and entertaining gameplay... "
    "Will you be able to finish all levels of the Single Player Mode ?? "
    "Will you beat the BIG BOSS ??? Have a try, "
    "I wish you good luck for that.", smallFont);
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
                        optionMenuLoop();
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
                        if (menu_active_is(gameOverMenu, "NO"))
                            menu_next_item(gameOverMenu);
                        while (menu_active_is(gameOverMenu, "YES")) {
                            menu_next_item(gameOverMenu);
                            PuyoStarter myStarter(this,false,0);
                            myStarter.run(score1, score2, 0);
                            score1 += myStarter.leftPlayerWin();
                            score2 += myStarter.rightPlayerWin();
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

void PuyoCommander::optionMenuLoop()
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
  int divisor = 1;
  if (menu_active_is (singleGameMenu, kLevelMedium))
    divisor = 2;
  else if (menu_active_is (singleGameMenu, kLevelHard))
    divisor = 4;
    
  int levelArray[] = { 250, 150, 100, 60, 45, 32, 25, 20, 15, 12, 10, 0 };
  int score1 = 0;
  int score2 = 0;
  int lives  = 3;
  if (menu_active_is(gameOverMenu, "NO"))
    menu_next_item(gameOverMenu);
  while (menu_active_is(gameOverMenu, "YES") && (lives >= 0)) {
    menu_next_item(gameOverMenu);
    PuyoStory myStory(this, score2+1);
    myStory.loop();
    PuyoStarter myStarter(this, true, levelArray[score2] / divisor);
    myStarter.run(score1, score2, lives);
    score1 += myStarter.leftPlayerWin();
    score2 += myStarter.rightPlayerWin();
    if (!myStarter.rightPlayerWin())
      lives--;
    if (levelArray[score2] == 0) {
      printf("YOU FINISHED THE GAME ! ! ! !\n");
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
  menu_update (rulesMenu, display);
  menu_update (aboutMenu, display);
  menu_update (singleGameMenu, display);
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
    menu_draw (rulesMenu, display);
    menu_draw (aboutMenu, display);
    menu_draw (singleGameMenu, display);
    SDL_Flip (display);
  }

  // delay si machine trop rapide
  now = SDL_GetTicks ();
  if (now < (start_time + cycle * cycle_duration))
    SDL_Delay ((start_time + cycle * cycle_duration) - now);
}


void PuyoCommander::initControllers()
{
    numJoysticks = SDL_NumJoysticks();
    for( int i=0 ; i < numJoysticks ; i++ ) 
    {
        joystick[i] = SDL_JoystickOpen(i);
    }
  SDL_JoystickEventState(SDL_ENABLE);
}

void PuyoCommander::closeControllers()
{
    for( int i=0 ; i < numJoysticks ; i++ ) 
    {
        SDL_JoystickClose(joystick[i]);
    }
}

void PuyoCommander::getControlEvent(SDL_Event e, GameControlEvent *result)
{
    result->gameEvent = GameControlEvent::kGameNone;
    result->cursorEvent = GameControlEvent::kCursorNone;
    
    switch (e.type) {
        case SDL_QUIT:
            result->cursorEvent = GameControlEvent::kQuit;
            break;
        case SDL_JOYBUTTONDOWN:
            switch (e.jbutton.button) {
                case 0:
                    result->cursorEvent = GameControlEvent::kStart;
                    if (e.jbutton.which == 0) {
                        result->gameEvent = GameControlEvent::kPlayer1TurnLeft;
                    }
                    else {
                        result->gameEvent = GameControlEvent::kPlayer2TurnLeft;
                    }
                    break;
                case 1:
                    result->cursorEvent = GameControlEvent::kBack;
                    if (e.jbutton.which == 0) {
                        result->gameEvent = GameControlEvent::kPlayer1TurnRight;
                    }
                    else {
                        result->gameEvent = GameControlEvent::kPlayer2TurnRight;
                    }
                    break;
            }
            break;
        case SDL_JOYAXISMOTION:
            if (e.jaxis.which == 0) {
                result->gameEvent = GameControlEvent::kPlayer1DownUp;
            }
            else {
                result->gameEvent = GameControlEvent::kPlayer2DownUp;
            }
            if ((e.jaxis.value < -3200) || (e.jaxis.value > 3200)) {
                if (e.jaxis.axis == 0) {
                    if (e.jaxis.value < 0) {
                        result->cursorEvent = GameControlEvent::kLeft;
                        if (e.jaxis.which == 0) {
                            result->gameEvent = GameControlEvent::kPlayer1Left;
                        }
                        else {
                            result->gameEvent = GameControlEvent::kPlayer2Left;
                        }
                    }
                    else {
                        result->cursorEvent = GameControlEvent::kRight;
                        if (e.jaxis.which == 0) {
                            result->gameEvent = GameControlEvent::kPlayer1Right;
                        }
                        else {
                            result->gameEvent = GameControlEvent::kPlayer2Right;
                        }
                    }
                }
                if (e.jaxis.axis == 1) {
                    if (e.jaxis.value < 0) {
                        result->cursorEvent = GameControlEvent::kUp;
                    }
                    else {
                        result->cursorEvent = GameControlEvent::kDown;
                        if (e.jaxis.which == 0) {
                            result->gameEvent = GameControlEvent::kPlayer1Down;
                        }
                        else {
                            result->gameEvent = GameControlEvent::kPlayer2Down;
                        }
                    }
                }
            }
            break;
        case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {
                case SDLK_DOWN:
                    result->cursorEvent = GameControlEvent::kDown;
                    result->gameEvent = GameControlEvent::kPlayer2Down;
                    break;
                case SDLK_UP:
                    result->cursorEvent = GameControlEvent::kUp;
                    result->gameEvent = GameControlEvent::kPlayer2TurnLeft;
                    break;
                case SDLK_LEFT:
                    result->gameEvent = GameControlEvent::kPlayer2Left;
                    break;
                case SDLK_RIGHT:
                    result->gameEvent = GameControlEvent::kPlayer2Right;
                    break;
                case SDLK_RETURN:
                    result->cursorEvent = GameControlEvent::kStart;
                    break;
                case SDLK_ESCAPE:
                    result->cursorEvent = GameControlEvent::kBack;
                    break;
                case SDLK_s:
                    result->gameEvent = GameControlEvent::kPlayer1Left;
                    break;
                case SDLK_f:
                    result->gameEvent = GameControlEvent::kPlayer1Right;
                    break;
                case SDLK_e:
                    result->gameEvent = GameControlEvent::kPlayer1TurnLeft;
                    break;
                case SDLK_d:
                    result->gameEvent = GameControlEvent::kPlayer1Down;
                    break;
                default:
                    break;
            }
            break;
        case SDL_KEYUP:
            switch (e.key.keysym.sym) {
                case SDLK_DOWN:
                    result->gameEvent = GameControlEvent::kPlayer2DownUp;
                    break;
                case SDLK_d:
                    result->gameEvent = GameControlEvent::kPlayer1DownUp;
                    break;
                default:
                    break;
            }
            break;
    }
}

