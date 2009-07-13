#include <iostream>
#include "PuyoMain.h"
#include "PuyoStrings.h"
#include "preferences.h"
#include "MainMenu.h"
#ifdef SDL12_GFX
#include "sdl_drawcontext/sdl12/sdl12_drawcontext.h"
#endif
#ifdef SDL13_GFX
#include "sdl_drawcontext/sdl13/sdl13_drawcontext.h"
#endif
#ifdef ENABLE_NETWORK_INTERNET
#include "PuyoInternetBot.h"
#endif

using namespace std;

// To be moved elsewhere
static const char * kFullScreenPref = "Config.FullScreen";
#ifdef HAVE_OPENGL
static const char * kOpenGLPref     = "Config.OpenGL";
#endif
static const char * kScreenWidthPref = "Config.ScreenWidth";
static const char * kScreenHeightPref = "Config.ScreenHeight";


PuyoMain::PuyoMain(String dataDir, bool fullscreen, int maxDataPackNumber)
: m_dataDir(dataDir), m_fullscreen(fullscreen), m_maxDataPackNumber(maxDataPackNumber)
{}

PuyoMain::~PuyoMain()
{
    gameui::GlobalNotificationCenter.removeListener(theCommander->getFullScreenKey(), this);
}

void PuyoMain::initWithGUI()
{
    initSDL();
    loop = GameUIDefaults::GAME_LOOP;
    // Create the DrawContext
    int requestedWidth = GetIntPreference(kScreenWidthPref, 640);
    int requestedHeight = GetIntPreference(kScreenHeightPref, 480);
#ifdef SDL12_GFX
    m_drawContext = new SDL12_DrawContext(640, 480,
                                          GetBoolPreference(kFullScreenPref, m_fullscreen),
                                          "FloboPuyo by iOS-Software");
#endif
#ifdef SDL13_GFX
    m_drawContext = new SDL13_DrawContext(640, 480,
                                          GetBoolPreference(kFullScreenPref, m_fullscreen),
                                          "FloboPuyo by iOS-Software");
#endif
    // Give the DrawContext to the GameLoop
    loop->setDrawContext(m_drawContext);
    // Create the PuyoCommander singleton
    PuyoCommander *pc = new PuyoCommander(m_dataDir, m_maxDataPackNumber);
    pc->initWithGUI(m_fullscreen);
    initMenus();
    cursor = new GameCursor(theCommander->getDataPathManager().getPath("gfx/cursor.png"));
    loop->addDrawable(cursor);
    loop->addIdle(cursor);
    theCommander->registerCursor(cursor);
    gameui::GlobalNotificationCenter.addListener(theCommander->getFullScreenKey(),this);
}

void PuyoMain::initWithoutGUI()
{
    loop = GameUIDefaults::GAME_LOOP;
    PuyoCommander *pc = new PuyoCommander(m_dataDir, m_maxDataPackNumber);
    pc->initWithoutGUI();
}

/* Initialize SDL context */
void PuyoMain::initSDL()
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
    if (theCommander->getFullScreen())
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

void PuyoMain::run()
{
  initWithGUI();
  GameUIDefaults::SCREEN_STACK->push(mainScreen);
  GameUIDefaults::GAME_LOOP->run();
}

void PuyoMain::debug_gsl(String gsl_script)
{
  initWithGUI();
  GameUIDefaults::SCREEN_STACK->push(mainScreen);
  StoryScreen story_screen(gsl_script);
  GameUIDefaults::SCREEN_STACK->push(&story_screen);
  GameUIDefaults::GAME_LOOP->run();
}

void PuyoMain::connect_ia(String param)
{
#ifdef ENABLE_NETWORK_INTERNET
  String name   = "Herbert";
  String server = "aley.fovea.cc";
  int port      = 4567;
  int what = 1;
  int first = 0;
  for (int i=0; i<=param.length(); ++i) {
      if ((param[i] == 0) || (param[i] == ':')) {
          if (what == 1) {
              name = param.substring(first, i);
              first = i+1;
              what++;
          } else if (what == 2) {
              server = param.substring(first, i);
              first = i+1;
              what++;
          } else if (what == 3) {
              port = atoi(param.substring(first, i).c_str());
              first = i+1;
              what++;
          }
      }
  }
  std::cout << name.c_str() << ":" << server.c_str() << ":" << port << std::endl;
  int level     = 1;
  String password = "";

  initWithoutGUI();

  PuyoInternetBot bot(level);
  bot.connect(server, port, name, password);

  GameUIDefaults::GAME_LOOP->addIdle(&bot);
  GameUIDefaults::GAME_LOOP->run();
  GameUIDefaults::GAME_LOOP->removeIdle(&bot);
#endif
}

void PuyoMain::initMenus()
{
  DBG_PRINT("initMenus()\n");
  //
  // Create the structures.
  StoryWidget *fgStory = new StoryWidget("title_fg.gsl");
  StoryWidget *bgStory = new StoryWidget("title_bg.gsl");
  mainScreen = new MainScreen(fgStory, bgStory);
  MainRealMenu *trubudu = new MainRealMenu(mainScreen);
  trubudu->build();
  mainScreen->pushMenu(trubudu);
}

void PuyoMain::notificationOccured(String identifier, void * context)
{
    if (identifier == theCommander->getFullScreenKey()) {
        SetBoolPreference(kFullScreenPref, *(bool *)context);
#ifdef SDL12_GFX
        static_cast<SDL12_DrawContext *>(m_drawContext)->setFullScreen(*(bool *)context);
#endif
#ifdef SDL13_GFX
        static_cast<SDL13_DrawContext *>(m_drawContext)->setFullScreen(*(bool *)context);
#endif
    }
}

