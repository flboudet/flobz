#include "PuyoMain.h"
#include "PuyoStrings.h"
#include "preferences.h"
#include "MainMenu.h"
#include "sdl12_drawcontext.h"

// To be moved elsewhere
static const char * kFullScreenPref = "Config.FullScreen";
#ifdef HAVE_OPENGL
static const char * kOpenGLPref     = "Config.OpenGL";
#endif
static const char * kScreenWidthPref = "Config.ScreenWidth";
static const char * kScreenHeightPref = "Config.ScreenHeight";


PuyoMain::PuyoMain(String dataDir, bool fullscreen, int maxDataPackNumber)
{
    initSDL();
    loop = GameUIDefaults::GAME_LOOP;
    // Create the DrawContext
    int requestedWidth = GetIntPreference(kScreenWidthPref, 640);
    int requestedHeight = GetIntPreference(kScreenHeightPref, 480);
    m_drawContext = new SDL12_DrawContext(640, 480,
                                          GetIntPreference(kFullScreenPref, fullscreen),
                                          "FloboPuyo by iOS-Software");
    // Give the DrawContext to the GameLoop
    loop->setDrawContext(m_drawContext);
    // Create the PuyoCommander singleton
    new PuyoCommander(dataDir, fullscreen, maxDataPackNumber);
    initMenus();
    cursor = new GameCursor(theCommander->getDataPathManager().getPath("gfx/cursor.png"));
    loop->addDrawable(cursor);
    loop->addIdle(cursor);
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

/* Init SDL display */
void PuyoMain::initDisplay(int w, int h, bool fullscreen, bool useGL)
{

}

void PuyoMain::run()
{
  GameUIDefaults::SCREEN_STACK->push(mainScreen);
  GameUIDefaults::GAME_LOOP->run();
}

void PuyoMain::debug_gsl(String gsl_script)
{
  GameUIDefaults::SCREEN_STACK->push(mainScreen);
  StoryScreen story_screen(gsl_script);
  GameUIDefaults::SCREEN_STACK->push(&story_screen);
  GameUIDefaults::GAME_LOOP->run();
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


