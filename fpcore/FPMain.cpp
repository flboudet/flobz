#include <iostream>
#include "FPMain.h"
#include "FPStrings.h"
#include "MainMenu.h"
#include "CompositeDrawContext.h"
#include "PackageDescription.h"
#ifdef SDL12_GFX
#ifdef OPENGL_GFX
#include "sdl_drawcontext/sdl12/sdl12_opengl_drawcontext.h"
#endif
#include "sdl_drawcontext/sdl12/sdl12_drawcontext.h"
#include "sdl_drawcontext/sdl12/sdl12_eventmanager.h"
#include "sdl_drawcontext/common/SDL_AudioManager.h"
#endif
#ifdef SDL13_GFX
#include "sdl_drawcontext/sdl13/sdl13_drawcontext.h"
#include "sdl_drawcontext/sdl13/sdl13_eventmanager.h"
#include "sdl_drawcontext/common/SDL_AudioManager.h"
#endif
#ifdef NULL_BACKENDS
#include "SlaveEventManager.h"
#include "NullDrawContext.h"
#include "NullAudioManager.h"
#endif

#ifdef ENABLE_NETWORK_INTERNET
#include "InternetBot.h"
#endif

#include "PosixPreferencesManager.h"

using namespace std;
using namespace event_manager;

// To be moved elsewhere
static const char * kFullScreenPref = "Config.FullScreen";
#ifdef HAVE_OPENGL
static const char * kOpenGLPref     = "Config.OpenGL";
#endif
static const char * kScreenWidthPref = "Config.ScreenWidth";
static const char * kScreenHeightPref = "Config.ScreenHeight";


FPMain::FPMain(String dataDir, bool fullscreen, int maxDataPackNumber)
: m_dataDir(dataDir), m_fullscreen(fullscreen),
  m_maxDataPackNumber(maxDataPackNumber),
  m_dataPathManager(dataDir)
{
    if (maxDataPackNumber != -1)
        m_dataPathManager.setMaxPackNumber(maxDataPackNumber);
}

FPMain::~FPMain()
{
    gameui::GlobalNotificationCenter.removeListener(theCommander->getFullScreenKey(), this);
}

void FPMain::initWithGUI()
{
    std::cout << "Initing SDL!\n";
    initSDL();
    loop = GameUIDefaults::GAME_LOOP;
    // Create the Preferences Manager
    std::string prefFilePath;
#ifndef _WIN32
    char * h = getenv("HOME");
    if (h != NULL) {
        prefFilePath = h;
        prefFilePath += "/.flobopoprc";
    }
#else
    // TODO: a better path for WIN32
    prefFilePath = "./.flobopoprc";
#endif
    std::cout << "prefmgr!\n";
    m_preferencesManager = new PosixPreferencesManager(prefFilePath.c_str());
    // Create the DrawContext
    int requestedWidth = m_preferencesManager->getIntPreference(kScreenWidthPref, 640);
    int requestedHeight = m_preferencesManager->getIntPreference(kScreenHeightPref, 480);
#ifdef SDL12_GFX
#ifdef OPENGL_GFX
    m_nativeDrawContext = new SDL12_OpenGL_DrawContext(&m_dataPathManager, 640, 480,
                                          m_preferencesManager->getBoolPreference(kFullScreenPref, m_fullscreen),
                                          "FloboPop by iOS-Software");
#else
    std::cout << "drawcontext!\n";
    m_nativeDrawContext = new SDL12_DrawContext(m_dataPathManager, 640, 480,
                                          m_preferencesManager->getBoolPreference(kFullScreenPref, m_fullscreen),
                                          "FloboPop by iOS-Software");
    std::cout << "drawcontext2!\n";
#endif
    //PackageDescription *packDesc = new PackageDescription(m_dataPathManager, *cDC);
    //IosRect cropRect = {0,0,32,32};
    //cDC->declareCompositeSurface("data/base.000/theme/Classic.fptheme/fat-flobo-0000.png",
    //                             "data/base.000/theme/Classic.fptheme/montage_1.png",
    //                             cropRect);
    std::cout << "eventmanager\n";
    m_eventManager = new SDL12_EventManager(m_preferencesManager);
    std::cout << "audiomanager\n";
    m_audioManager = new SDL_AudioManager();
    std::cout << "audiomanager2\n";
#endif
#ifdef SDL13_GFX
    m_nativeDrawContext = new SDL13_DrawContext(640, 480,
                                          m_preferencesManager->getBoolPreference(kFullScreenPref, m_fullscreen),
                                          "FloboPop by iOS-Software");
    m_eventManager = new SDL13_EventManager();
    m_audioManager = new SDL_AudioManager();
#endif
#ifdef NULL_BACKENDS
    m_nativeDrawContext = new NullDrawContext(640, 480);
    m_eventManager = new SlaveEventManager();
    m_audioManager = new NullAudioManager();
#endif
    std::cout << "compositedrawcontext!\n";
    m_drawContext = new CompositeDrawContext(m_nativeDrawContext);
    // Give the DrawContext to the GameLoop
    loop->setDrawContext(m_drawContext);
    // Give the EventManager to the GameLoop
    loop->setEventManager(m_eventManager);
    // Give the AudioManager to the GameLoop
    loop->setAudioManager(m_audioManager);
    std::cout << "suite!\n";
    // Register data packages
    m_dataPathManager.registerDataPackages(m_drawContext, &m_jukebox);
    // Create the FPCommander singleton
    FPCommander *pc = new FPCommander(&m_dataPathManager, m_preferencesManager, &m_jukebox);
    std::cout << "fpcommander!\n";
    pc->initWithGUI(m_fullscreen);
    std::cout << "fpcommander2!\n";
    initMenus();
    std::cout << "fpcommander3!\n";
    cursor = new GameCursor("gfx/cursor.png");
    std::cout << "fpcommander4!\n";
    loop->addDrawable(cursor);
    loop->addIdle(cursor);
    std::cout << "idle!\n";
    if (dynamic_cast<CycledComponent *>(m_eventManager) != NULL)
        loop->addIdle(dynamic_cast<CycledComponent *>(m_eventManager));
    theCommander->registerCursor(cursor);
    gameui::GlobalNotificationCenter.addListener(theCommander->getFullScreenKey(),this);
    std::cout << "done!\n";
}

#ifdef SDL13_GFX
#define SDL_GFX
#endif
#ifdef SDL12_GFX
#define SDL_GFX
#endif

/* Initialize SDL context */
void FPMain::initSDL()
{
#ifdef SDL_GFX
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
#endif
}

void FPMain::run()
{
    std::cout << "fpmain run()\n";
  initWithGUI();
  std::cout << "Initialized!\n";
  GameUIDefaults::SCREEN_STACK->push(mainScreen);
  std::cout << "Pushed!\n";
}

void FPMain::debug_gsl(String gsl_script)
{
  initWithGUI();
  GameUIDefaults::SCREEN_STACK->push(mainScreen);
  StoryScreen story_screen(gsl_script);
  GameUIDefaults::SCREEN_STACK->push(&story_screen);
  GameUIDefaults::GAME_LOOP->run();
}

void FPMain::connect_ia(String param)
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

  //initWithoutGUI();

  InternetBot bot(level);
  bot.connect(server, port, name, password);

  GameUIDefaults::GAME_LOOP->addIdle(&bot);
  GameUIDefaults::GAME_LOOP->run();
  GameUIDefaults::GAME_LOOP->removeIdle(&bot);
#endif
}

void FPMain::initMenus()
{
    std::cout << "InitMenus!\n";
  DBG_PRINT("initMenus()\n");
  //
  // Create the structures.
  StoryWidget *fgStory = new StoryWidget("title_fg.gsl");
  std::cout << "story!\n";
  StoryWidget *bgStory = new StoryWidget("title_bg.gsl");
  std::cout << "story2!\n";
  mainScreen = new MainScreen(fgStory, bgStory);
  std::cout << "mainScreen!\n";
  MainRealMenu *trubudu = new MainRealMenu(mainScreen);
  trubudu->build();
  mainScreen->pushMenu(trubudu);
}

void FPMain::notificationOccured(String identifier, void * context)
{
    if (identifier == theCommander->getFullScreenKey()) {
        theCommander->getPreferencesManager()->setBoolPreference(kFullScreenPref, *(bool *)context);
#ifdef SDL12_GFX
#ifdef OPENGL_GFX
        static_cast<SDL12_OpenGL_DrawContext *>(m_nativeDrawContext)->setFullScreen(*(bool *)context);
#else
        static_cast<SDL12_DrawContext *>(m_nativeDrawContext)->setFullScreen(*(bool *)context);
#endif
#endif
#ifdef SDL13_GFX
        static_cast<SDL13_DrawContext *>(m_nativeDrawContext)->setFullScreen(*(bool *)context);
#endif
    }
}
