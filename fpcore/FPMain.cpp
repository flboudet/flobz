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
#ifdef SDL2_GFX
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


FPMain::FPMain(const std::string &dataDir, bool fullscreen, int maxDataPackNumber)
: _dataDir(dataDir), _fullscreen(fullscreen),
  _maxDataPackNumber(maxDataPackNumber),
  _dataPathManager(dataDir)
{
    if (maxDataPackNumber != -1)
        _dataPathManager.setMaxPackNumber(maxDataPackNumber);
}

FPMain::~FPMain()
{
    gameui::GlobalNotificationCenter.removeListener(theCommander->getFullScreenKey(), this);
}

void FPMain::initWithGUI()
{
    std::cout << "Initing SDL!\n";
    initSDL();
    _loop = GameUIDefaults::GAME_LOOP;
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
    _preferencesManager = new PosixPreferencesManager(prefFilePath.c_str());
    // Create the DrawContext
    int requestedWidth = _preferencesManager->getIntPreference(kScreenWidthPref, 640);
    int requestedHeight = _preferencesManager->getIntPreference(kScreenHeightPref, 480);
#ifdef SDL12_GFX
#ifdef OPENGL_GFX
    _nativeDrawContext = new SDL12_OpenGL_DrawContext(&_dataPathManager, 640, 480,
                                          _preferencesManager->getBoolPreference(kFullScreenPref, _fullscreen),
                                          "FloboPop by iOS-Software");
#else
    std::cout << "drawcontext!\n";
    _nativeDrawContext = new SDL12_DrawContext(_dataPathManager, 640, 480,
                                          _preferencesManager->getBoolPreference(kFullScreenPref, _fullscreen),
                                          "FloboPop by iOS-Software");
    std::cout << "drawcontext2!\n";
#endif
    //PackageDescription *packDesc = new PackageDescription(_dataPathManager, *cDC);
    //IosRect cropRect = {0,0,32,32};
    //cDC->declareCompositeSurface("data/base.000/theme/Classic.fptheme/fat-flobo-0000.png",
    //                             "data/base.000/theme/Classic.fptheme/montage_1.png",
    //                             cropRect);
    std::cout << "eventmanager\n";
    _eventManager = new SDL12_EventManager(_preferencesManager);
    std::cout << "audiomanager\n";
    _audioManager = new SDL_AudioManager();
    std::cout << "audiomanager2\n";
#endif
#ifdef SDL2_GFX
    _nativeDrawContext = new SDL13_DrawContext(_dataPathManager, 640, 480,
                                          _preferencesManager->getBoolPreference(kFullScreenPref, _fullscreen),
                                          "FloboPop by iOS-Software");
    _eventManager = new SDL13_EventManager();
    _audioManager = new SDL_AudioManager();
#endif
#ifdef NULL_BACKENDS
    _nativeDrawContext = new NullDrawContext(640, 480);
    _eventManager = new SlaveEventManager();
    _audioManager = new NullAudioManager();
#endif
    std::cout << "compositedrawcontext!\n";
    _drawContext = new CompositeDrawContext(_nativeDrawContext);
    // Give the DrawContext to the GameLoop
    _loop->setDrawContext(_drawContext);
    // Give the EventManager to the GameLoop
    _loop->setEventManager(_eventManager);
    // Give the AudioManager to the GameLoop
    _loop->setAudioManager(_audioManager);
    std::cout << "suite!\n";
    // Register data packages
    _dataPathManager.registerDataPackages(_drawContext, &_jukebox);
    // Create the FPCommander singleton
    FPCommander *pc = new FPCommander(&_dataPathManager, _preferencesManager, &_jukebox);
    std::cout << "fpcommander!\n";
    pc->initWithGUI(_fullscreen);
    std::cout << "fpcommander2!\n";
    initMenus();
    std::cout << "fpcommander3!\n";
    _cursor = new GameCursor("gfx/cursor.png");
    std::cout << "fpcommander4!\n";
    _loop->addDrawable(_cursor);
    _loop->addIdle(_cursor);
    std::cout << "idle!\n";
    if (dynamic_cast<CycledComponent *>(_eventManager) != NULL)
        _loop->addIdle(dynamic_cast<CycledComponent *>(_eventManager));
    theCommander->registerCursor(_cursor);
    gameui::GlobalNotificationCenter.addListener(theCommander->getFullScreenKey(),this);
    std::cout << "done!\n";
}

#ifdef SDL2_GFX
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
  if (_fullscreen)
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
  GameUIDefaults::SCREEN_STACK->push(_mainScreen);
  std::cout << "Pushed!\n";
}

void FPMain::debug_gsl(const std::string &gsl_script)
{
  initWithGUI();
  GameUIDefaults::SCREEN_STACK->push(_mainScreen);
  StoryScreen story_screen(gsl_script);
  GameUIDefaults::SCREEN_STACK->push(&story_screen);
  GameUIDefaults::GAME_LOOP->run();
}

void FPMain::connect_ia(const std::string & param)
{
#ifdef ENABLE_NETWORK_INTERNET
  std::string & name   = "Herbert";
  std::string & server = "aley.fovea.cc";
  int port      = 4567;
  int what = 1;
  int first = 0;
  for (int i=0; i<=param.length(); ++i) {
      if ((param[i] == 0) || (param[i] == ':')) {
          if (what == 1) {
              name = param.substr(first, i - first);
              first = i+1;
              what++;
          } else if (what == 2) {
              server = param.substr(first, i - first);
              first = i+1;
              what++;
          } else if (what == 3) {
              port = atoi(param.substr(first, i - first).c_str());
              first = i+1;
              what++;
          }
      }
  }
  std::cout << name.c_str() << ":" << server.c_str() << ":" << port << std::endl;
  int level     = 1;
  std::string & password = "";

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
  _mainScreen = new MainScreen(fgStory, bgStory);
  std::cout << "_mainScreen!\n";
  MainRealMenu *trubudu = new MainRealMenu(_mainScreen);
  trubudu->build();
  _mainScreen->pushMenu(trubudu);
}

void FPMain::notificationOccured(const std::string & identifier, void * context)
{
    if (identifier == theCommander->getFullScreenKey()) {
        theCommander->getPreferencesManager()->setBoolPreference(kFullScreenPref, *(bool *)context);
#ifdef SDL12_GFX
#ifdef OPENGL_GFX
        static_cast<SDL12_OpenGL_DrawContext *>(_nativeDrawContext)->setFullScreen(*(bool *)context);
#else
        static_cast<SDL12_DrawContext *>(_nativeDrawContext)->setFullScreen(*(bool *)context);
#endif
#endif
#ifdef SDL2_GFX
        static_cast<SDL13_DrawContext *>(_nativeDrawContext)->setFullScreen(*(bool *)context);
#endif
    }
}
