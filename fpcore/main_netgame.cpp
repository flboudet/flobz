#include <stdlib.h>
#include <string.h>
#ifdef MACOSX
#include <CoreFoundation/CoreFoundation.h>
#include "SDL_main.h"
#else
//#include <SDL_main.h>
#endif

#include "ios_exception.h"
#include "ios_memory.h"
#include "ios_filepath.h"

#ifndef DATADIR
static const char *DATADIR = "data";
#endif

using namespace ios_fc;

#include "MainScreen.h"
#include "FPCommander.h"
#include "PreferencesManager.h"
#include "PosixPreferencesManager.h"
#include "FPDataPathManager.h"
#include "drawcontext.h"
#include "CompositeDrawContext.h"
#include "audio.h"

#include "sdl_drawcontext/sdl13/sdl13_drawcontext.h"
#include "sdl_drawcontext/sdl13/sdl13_eventmanager.h"
#include "sdl_drawcontext/common/SDL_AudioManager.h"

static const char * kScreenWidthPref = "Config.ScreenWidth";
static const char * kScreenHeightPref = "Config.ScreenHeight";
static const char * kFullScreenPref = "Config.FullScreen";

class FPNetMain : NotificationResponder
{
public:
    FPNetMain(const std::string &dataDir, bool fullscreen, int maxDataPackNumber=-1);
    virtual ~FPNetMain();
    void run();
protected:
    void notificationOccured(String identifier, void * context);
private:
    void initWithGUI();
    void initMenus();
    void initSDL();

    std::string _dataDir;
    bool _fullscreen;
    int _maxDataPackNumber;

    PreferencesManager *m_preferencesManager;
    CompositeDrawContext *m_drawContext;
    DrawContext *m_nativeDrawContext;
    event_manager::EventManager *m_eventManager;
    audio_manager::AudioManager *m_audioManager;

    Jukebox m_jukebox;

    FPDataPathManager _dataPathManager;

    GameLoop   *loop;
    MainScreen *mainScreen;
    GameCursor *cursor;
};

FPNetMain::FPNetMain(const std::string &dataDir, bool fullscreen, int maxDataPackNumber)
: _dataDir(dataDir), _fullscreen(fullscreen),
  _maxDataPackNumber(maxDataPackNumber),
  _dataPathManager(dataDir)
{
    if (maxDataPackNumber != -1)
        _dataPathManager.setMaxPackNumber(maxDataPackNumber);
}

FPNetMain::~FPNetMain()
{
    gameui::GlobalNotificationCenter.removeListener(theCommander->getFullScreenKey(), this);
}

void FPNetMain::initWithGUI()
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

    m_nativeDrawContext = new SDL13_DrawContext(_dataPathManager, 640, 480,
                                          m_preferencesManager->getBoolPreference(kFullScreenPref, _fullscreen),
                                          "FloboPop by iOS-Software");
    m_eventManager = new SDL13_EventManager();
    m_audioManager = new SDL_AudioManager();


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
    _dataPathManager.registerDataPackages(m_drawContext, &m_jukebox);
    // Create the FPCommander singleton
    FPCommander *pc = new FPCommander(&_dataPathManager, m_preferencesManager, &m_jukebox);
    std::cout << "fpcommander!\n";
    pc->initWithGUI(_fullscreen);
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

void FPNetMain::initMenus()
{
}

#ifdef SDL2_GFX
#define SDL_GFX
#endif
#ifdef SDL12_GFX
#define SDL_GFX
#endif

/* Initialize SDL context */
void FPNetMain::initSDL()
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

void FPNetMain::run()
{
    std::cout << "fpnetmain run()\n";
  initWithGUI();
  std::cout << "Initialized!\n";
  GameUIDefaults::SCREEN_STACK->push(mainScreen);
  std::cout << "Pushed!\n";
}

bool fileExists(char *path)
{
    FILE *f;
    f = fopen(path, "r");
    if (f == NULL)
        return false;
    fclose(f);
    return true;
}


static void displayExceptionMessage(const char * msg)
{
  cout << "Sorry, an unhandled exception occured. Giving up." << endl;
  cout << msg << endl;
  cout << "You can report this problem to ios@ios-software.com" << endl;
}

static void help()
{
    cout << "usage: ./flobopop [options]" << endl;
    cout << endl;
    cout << "options are:" << endl;
    cout << "  -fs                           enable fullscreen mode" << endl;
    cout << "  -maxpack {n}                  use data pack n (0..2)" << endl;
    cout << "  -gsl {file}                   launch a gsl script" << endl;
    cout << "  -ia {name}:{server}:{port}    connect a bot to a server" << endl;
    cout << endl;
    cout << "(c)2007, by iOS-Software (G.Borios, F.Boudet, J.C.Hoelt)." << endl;
    exit(0);
}

int main(int argc, char *argv[])
{
    int i;
    std::string dataDir("data");
    int maxPackNumber = -1;

    bool fs = false;

    FPNetMain fp(dataDir, fs, maxPackNumber);
#ifdef DISABLED
    try {
        fp.run();
        GameUIDefaults::GAME_LOOP->run();
    }
    catch (Exception e) { displayExceptionMessage(e.what()); }
    catch (char * str) { displayExceptionMessage(str); }
    catch (String str) { displayExceptionMessage(str); }
    catch (...) { displayExceptionMessage("The exception is unknown."); }
#endif
    return 0;
}
