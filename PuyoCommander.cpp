/* strings to translate */

#include "PuyoCommander.h"
#include "PuyoStrings.h"
#include "gameui.h"
#include "preferences.h"
#include "audio.h"
#include "PuyoSinglePlayerStarter.h"

#include "PuyoNetworkStarter.h"
#include "ios_udpmessagebox.h"
using namespace gameui;

PuyoCommander *theCommander = NULL;
IIM_Surface   *menuBG       = NULL;
SoFont *storyFont;
#define WIDTH  640
#define HEIGHT 480

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

class NetGameAction : public Action {
  public: void action();
};

/*
 * THE MENUS
 */
class PuyoScreen : public Screen {
  public:
    PuyoScreen() : Screen(0,0,WIDTH,HEIGHT) { setBackground(menuBG); }
    virtual ~PuyoScreen() {}
    virtual void build() = 0;
};

class MainMenu : public PuyoScreen {
  public: void build();
};

class NetworkGameMenu : public PuyoScreen {
  public: void build();
};

void MainMenu::build() {
  add(new Button(kSinglePlayerGame, new SinglePlayerGameAction));
  add(new Button(kNetGame, new PushScreenAction(theCommander->netGameMenu)));
  add(new Button(kExit,    new ExitAction));
}

void NetworkGameMenu::build() {
  add(new Button("GO!", new NetGameAction));
  add(new EditField("Player"));
  add(new Button("Cancel", new PopScreenAction()));
}

/**
 * Launches a single player game
 */
void SinglePlayerGameAction::action()
{
  PuyoStarter *starter = new PuyoSinglePlayerStarter(theCommander, 5, FLOBO, 0);
  GameUIDefaults::SCREEN_STACK->push(starter);
  starter->run(0,0,0,0,0);
  GameUIDefaults::SCREEN_STACK->pop();
  starter->kill();
}

/**
 * Launches a network game
 */
void NetGameAction::action()
{
  UDPMessageBox mbox("127.0.0.1", 6581, 6581);
  PuyoStarter *starter = new PuyoNetworkStarter(theCommander, 0, &mbox);
  GameUIDefaults::SCREEN_STACK->push(starter);
  starter->run(0,0,0,0,0);
  GameUIDefaults::SCREEN_STACK->pop();
  starter->kill();
}

void PuyoCommander::run()
{
  GameUIDefaults::SCREEN_STACK->push(mainMenu);
  while(1) {
    updateAll(NULL);
  }
}

void PuyoCommander::initMenus()
{
  menuBG = IIM_Load_DisplayFormat("MenuBackground.jpg");
  // 
  // Create the structures.
  mainMenu    = new MainMenu;
  netGameMenu = new NetworkGameMenu;

  // Build the menus.
  mainMenu->build();
  netGameMenu->build();
}

/* Build the PuyoCommander */

PuyoCommander::PuyoCommander(bool fs, bool snd, bool audio)
{
  SDL_Delay(500);
  loop = GameUIDefaults::GAME_LOOP;
  mbox = NULL;
  theCommander = this;

  loadPreferences(fs, snd, audio);
  initGameControls();
  initSDL();
  initAudio();
  initDisplay();
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


/* Initialize the audio if necessary */
void PuyoCommander::initAudio()
{
#ifdef USE_AUDIO
  DBG_PRINT("initAudio()\n");
  int music_volume = GetIntPreference(kMusicVolume, 100);
  int audio_volume = GetIntPreference(kAudioVolume, 80);

  audio_init();
  audio_music_start(0);
  if (sound==false) Mix_PauseMusic();
  audio_set_music_on_off(sound);
  audio_set_sound_on_off(fx);

  audio_set_volume(audio_volume);
  audio_music_set_volume(music_volume);
#endif
}


/* load fonts and set them for use in the GUI */
void PuyoCommander::initFonts()
{
  DBG_PRINT("initFonts()\n");
  smallFont = SoFont_new();
  SoFont_load (smallFont, IIM_Load_DisplayFormatAlpha ("font4b.png"));
  menuFont = SoFont_new();
  SoFont_load (menuFont, IIM_Load_DisplayFormatAlpha ("font3b.png"));
  darkFont = SoFont_new();
  SoFont_load (darkFont, IIM_Load_DisplayFormatAlpha ("fontdark.png"));
  storyFont = darkFont;
  
  GameUIDefaults::FONT          = menuFont;
  GameUIDefaults::FONT_INACTIVE = darkFont;
}


/* Init SDL display */
void PuyoCommander::initDisplay()
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
}


/* load a few important preferences for display and sound initialisation */
void PuyoCommander::loadPreferences(bool fs, bool snd, bool audio)
{  
  DBG_PRINT("loadPreferences()\n");
  /* Load Preferences */
  fullscreen = GetBoolPreference(kFullScreen, fs);
#ifdef HAVE_OPENGL
  useGL      = GetBoolPreference(kOpenGL,false);
#endif
  sound = GetBoolPreference(kMusic,snd);
  fx = GetBoolPreference(kAudioFX,audio);
}



void PuyoCommander::updateAll(DrawableComponent *starter, SDL_Surface *extra_surf)
{
  double currentTime = loop->getCurrentTime();
  loop->idle(currentTime);
  if (!loop->isLate(currentTime) && loop->drawRequested()) {
    loop->draw();
  }
}

void PuyoCommander::onMessage(Message &msb) {}
