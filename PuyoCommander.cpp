/* strings to translate */

#include "PuyoCommander.h"
#include "PuyoStrings.h"
#include "gameui.h"
#include "preferences.h"
#include "audio.h"
#include "PuyoSinglePlayerStarter.h"

#include "PuyoNetworkStarter.h"

#include "PuyoLocalMenu.h"
#include "PuyoNetworkMenu.h"
#include "PuyoOptionMenu.h"
#include "ios_udpmessagebox.h"

using namespace gameui;

PuyoCommander *theCommander = NULL;
IIM_Surface   *menuBG       = NULL;
IIM_Surface   *menuBG_wide  = NULL;
SoFont *storyFont;
#define WIDTH  640
#define HEIGHT 480

static const char * kFullScreenPref = "Config.FullScreen";
static const char * kOpenGLPref     = "Config.OpenGL";

/*
 * MENU ACTIONS
 */
class ExitAction : public Action {
  public:
    void action() { SDL_Quit(); exit(0); }
};

class SinglePlayerGameAction : public Action {
  public: void action();
};

void PuyoPushMenuAction::action()
{
    mainScreen->pushMenu(menu);
}

void PuyoPopMenuAction::action()
{
    mainScreen->popMenu();
}

/*
 * THE MENUS
 */
PuyoScreen::PuyoScreen() : Screen(0,0,WIDTH,HEIGHT) { setBackground(menuBG); }

PuyoMainScreen::PuyoMainScreen(PuyoStoryWidget *story) : story(story)
{
}

#define MENU_X 235
#define MENU_Y 225

void PuyoMainScreen::build()
{
    if (story != NULL)
        add(story);
    add(&menu);
    Vec3 menuPos = menu.getPosition();
    menuPos.x = MENU_X;
    menuPos.y = MENU_Y;
    menu.setPosition(menuPos);
    menu.setSize(Vec3(menuBG->w, menuBG->h, 0));
}

PuyoRealMainScreen::PuyoRealMainScreen(PuyoStoryWidget *story) : story(story), transition(NULL)
{
    if (story != NULL)
        add(story);
    add(&container);
    Vec3 menuPos = container.getPosition();
    menuPos.x = MENU_X;
    menuPos.y = MENU_Y;

    container.setPosition(menuPos);
    container.setSize(Vec3(menuBG->w, menuBG->h, 0));
    container.setBackground(menuBG);
}

PuyoRealMainScreen::~PuyoRealMainScreen()
{
    if (transition != NULL) {
        delete(transition);
    }
}

void PuyoRealMainScreen::pushMenu(PuyoMainScreenMenu *menu)
{
    menuStack.push(container.getContentWidget());
    container.transitionToContent(menu);
}

void PuyoRealMainScreen::popMenu()
{
    if (menuStack.size() == 1)
        return;
    container.transitionToContent(menuStack.top());
    menuStack.pop();
}

void PuyoRealMainScreen::transitionFromScreen(Screen &fromScreen)
{
    if (transition != NULL) {
        remove(transition);
        delete(transition);
    }
    transition = new PuyoScreenTransitionWidget(fromScreen, NULL);
    add(transition);
    Vec3 menuPos = container.getPosition();
    menuPos.x = MENU_X;
    menuPos.y = MENU_Y;

    container.setPosition(menuPos);
    container.setSize(Vec3(menuBG->w, menuBG->h, 0));
}

void PuyoRealMainScreen::onEvent(GameControlEvent *cevent)
{
    PuyoScreen::onEvent(cevent);
	if (cevent->caught != false)
		return;
    switch (cevent->cursorEvent) {
    case GameControlEvent::kStart:
        break;
    case GameControlEvent::kBack:
        popMenu();
        break;
    }
}

class MainRealMenu : public PuyoMainScreenMenu {
public:
    MainRealMenu(PuyoRealMainScreen * mainScreen)
      : PuyoMainScreenMenu(mainScreen), localGameMenu(mainScreen),
        local2PlayersGameMenu(mainScreen), optionMenu(mainScreen),
        networkGameMenu(mainScreen), locale(theCommander->getDataPathManager().getPath("locale/main")),
        singlePlayerGameAction(&localGameMenu, mainScreen), twoPlayersGameAction(&local2PlayersGameMenu, mainScreen),
        optionAction(&optionMenu, mainScreen), networkGameAction(&networkGameMenu, mainScreen),
        singlePlayerGameButton(locale.getLocalizedString(kSinglePlayerGame), &singlePlayerGameAction),
        twoPlayersGameButton(locale.getLocalizedString("Two Players Game"), &twoPlayersGameAction),
        optionButton(locale.getLocalizedString("Options"), &optionAction),
        networkGameButton(locale.getLocalizedString(kNetGame), &networkGameAction),
        exitButton(locale.getLocalizedString(kExit), &exitAction) {}
    void build();
private:
    LocalGameMenu localGameMenu;
    Local2PlayersGameMenu local2PlayersGameMenu;
    OptionMenu optionMenu;
    NetworkGameMenu networkGameMenu;
    PuyoLocalizedDictionary locale;
    PuyoPushMenuAction singlePlayerGameAction, twoPlayersGameAction, optionAction, networkGameAction;
    ExitAction exitAction;
    Button singlePlayerGameButton, twoPlayersGameButton, optionButton, networkGameButton, exitButton;
};

void MainRealMenu::build() {
  localGameMenu.build();
  local2PlayersGameMenu.build();
  optionMenu.build();
  networkGameMenu.build();
  add(&singlePlayerGameButton);
  add(&twoPlayersGameButton);
  add(&optionButton);
  add(&networkGameButton);
  add(&exitButton);
}

/**
 * Launches a single player game
 */
void SinglePlayerGameAction::action()
{
  /*PuyoStarter *starter = new PuyoSinglePlayerStarter(theCommander, 5, FLOBO, 0);
  starter->run(0,0,0,0,0);
  GameUIDefaults::SCREEN_STACK->push(starter);*/
}

void PuyoCommander::run()
{
  GameUIDefaults::SCREEN_STACK->push(mainScreen);
  GameUIDefaults::GAME_LOOP->run();
}

void PuyoCommander::initMenus()
{
  DBG_PRINT("initMenus()\n");
  menuBG = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/menubg.png"));
  menuBG_wide = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/menubg-wide.png"));
  // 
  // Create the structures.
  PuyoStoryWidget *tempStory = new PuyoStoryWidget("title.gsl");
  mainScreen = new PuyoRealMainScreen(tempStory);
  MainRealMenu *trubudu = new MainRealMenu(mainScreen);
  trubudu->build();
  mainScreen->pushMenu(trubudu);
}

/* Build the PuyoCommander */

PuyoCommander::PuyoCommander(String dataDir, bool fs, int maxDataPackNumber) : dataPathManager(dataDir)
{
  //SDL_Delay(500);
  loop = GameUIDefaults::GAME_LOOP;
  mbox = NULL;
  theCommander = this;

  if (maxDataPackNumber != -1)
    dataPathManager.setMaxPackNumber(maxDataPackNumber);
  loadPreferences(fs);
  initSDL();
  initGameControls();
  initAudio();
  initDisplay(fullscreen, useGL);
  initFonts();
  initMenus();
}


/* Initialize SDL context */
void PuyoCommander::initSDL()
{
  DBG_PRINT("initSDL()\n");
  int init_flags = SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK;

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
}

extern char *dataFolder;
#include <string>

/* Initialize the audio if necessary */
void PuyoCommander::initAudio()
{
#ifdef USE_AUDIO
  DBG_PRINT("initAudio()\n");
  AudioManager::init();
#endif
}


/* load fonts and set them for use in the GUI */
void PuyoCommander::initFonts()
{
  DBG_PRINT("initFonts()\n");

  IIM_Surface *font3b = IIM_Load_Absolute_DisplayFormatAlpha (dataPathManager.getPath("gfx/font3b.png"));
  IIM_Surface *font4b = IIM_Load_Absolute_DisplayFormatAlpha (dataPathManager.getPath("gfx/font4b.png"));
  IIM_Surface *font6b = iim_surface_shift_hsv(font4b, 170, -.5, -.2);
  IIM_Surface *font5b = iim_surface_shift_hsv(font3b, 170, -.5, -.2);

  SoFont *darkFont = SoFont_new();
  SoFont_load (darkFont, IIM_Load_Absolute_DisplayFormatAlpha (dataPathManager.getPath("gfx/fontdark.png")));
  
  storyFont = darkFont;

  menuFont = SoFont_new();
  SoFont_load (menuFont, font3b);

  smallFont = SoFont_new();
  SoFont_load (smallFont, font4b);


  smallFontInfo = SoFont_new();
  SoFont_load (smallFontInfo, font6b);


  SoFont *textFont = SoFont_new();
  SoFont_load(textFont, font5b);

  SoFont_free(GameUIDefaults::FONT);
  GameUIDefaults::FONT            = menuFont;
  SoFont_free(GameUIDefaults::FONT_TEXT);
  GameUIDefaults::FONT_TEXT       = textFont;
  SoFont_free(GameUIDefaults::FONT_INACTIVE);
  GameUIDefaults::FONT_INACTIVE   = darkFont;
  SoFont_free(GameUIDefaults::FONT_SMALL_INFO);
  GameUIDefaults::FONT_SMALL_INFO = smallFontInfo;
    SoFont_free(GameUIDefaults::FONT_SMALL_ACTIVE);
  GameUIDefaults::FONT_SMALL_ACTIVE = smallFont;
}


void PuyoCommander::setMusic(bool music)
{
  AudioManager::musicOnOff(music);
}
    
void PuyoCommander::setSoundFx(bool fx)
{
  AudioManager::soundOnOff(fx);
}

bool PuyoCommander::getMusic()
{
  return AudioManager::isMusicOn();
}

bool PuyoCommander::getSoundFx()
{
  return AudioManager::isSoundOn();
}

void PuyoCommander::setFullScreen(bool fullScreen)
{
    if (fullScreen != this->fullscreen) {
        this->fullscreen = fullScreen;
        SetBoolPreference(kFullScreenPref, fullscreen);
        if (SDL_WM_ToggleFullScreen(loop->getSurface())==0)
        {
          // This should not be necessary (and actually prevents Windows
          // going from fullscreen to windowed mode)
          //SDL_QuitSubSystem(SDL_INIT_VIDEO);
          //SDL_InitSubSystem(SDL_INIT_VIDEO);
          
          initDisplay(fullscreen, useGL);
        }
        /* Workaround for cursor showing in MacOS X fullscreen mode */
        SDL_ShowCursor(SDL_ENABLE);
        SDL_ShowCursor(SDL_DISABLE);
    }
}

void PuyoCommander::setGlSDL(bool useGL)
{
}
    
/* Init SDL display */
void PuyoCommander::initDisplay(bool fullscreen, bool useGL)
{
  DBG_PRINT("initDisplay()\n");
  display = SDL_SetVideoMode( 640, 480, 0,  SDL_ANYFORMAT|SDL_HWSURFACE|SDL_DOUBLEBUF|(fullscreen?SDL_FULLSCREEN:0)|(useGL?SDL_GLSDL:0));
  if ( display == NULL ) {
    fprintf(stderr, "SDL_SetVideoMode error: %s\n",
            SDL_GetError());
    exit(1);
  }
  loop->setSurface(display);
  atexit(SDL_Quit); 
  SDL_ShowCursor(SDL_DISABLE);
  SDL_WM_SetCaption("FloboPuyo by iOS-Software",NULL);
  /* Should also set up an icon someday */
  //SDL_WM_SetIcon(SDL_LoadBMP("icon.bmp"), NULL);
}


/* load a few important preferences for display */
void PuyoCommander::loadPreferences(bool fs)
{  
  DBG_PRINT("loadPreferences()\n");
  /* Load Preferences */
  fullscreen = fs ? GetBoolPreference(kFullScreenPref, true) : false;
#ifdef HAVE_OPENGL
  useGL      = GetBoolPreference(kOpenGLPref,false);
#endif
}

void PuyoCommander::onMessage(Message &msb) {}
