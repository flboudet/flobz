#include "PuyoMain.h"
#include "PuyoStrings.h"
#include "preferences.h"
#include "MainMenu.h"

// To be moved elsewhere
static const char * kFullScreenPref = "Config.FullScreen";
#ifdef HAVE_OPENGL
static const char * kOpenGLPref     = "Config.OpenGL";
#endif
static const char * kScreenWidthPref = "Config.ScreenWidth";
static const char * kScreenHeightPref = "Config.ScreenHeight";


PuyoMain::PuyoMain(String dataDir, bool fullscreen, int maxDataPackNumber)
{
    // Create the PuyoCommander singleton
    loop = GameUIDefaults::GAME_LOOP;
    initSDL();
    initDisplay(GetIntPreference(kScreenWidthPref, 640),
                GetIntPreference(kScreenHeightPref, 480), GetIntPreference(kFullScreenPref, fullscreen), useGL);
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
  DBG_PRINT("initDisplay()\n");

  if (useGL) {
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      if ((display = SDL_SetVideoMode(w, h, 24, SDL_OPENGL|(fullscreen?SDL_FULLSCREEN:0))) == NULL) {
          fprintf(stderr, "Couldn't set GL mode: %s\n", SDL_GetError());
          SDL_Quit();
          return;
      }
    Uint32 rmask, gmask, bmask, amask;
    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x00000000;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0x00000000;
#endif
      loop->setSurface(SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 32,rmask, gmask, bmask, amask));
      loop->setDisplay(display);
      loop->setOpenGLMode(true);
  }
  else {
      display = SDL_SetVideoMode(w, h, 0, SDL_ANYFORMAT|SDL_HWSURFACE|SDL_DOUBLEBUF|(fullscreen?SDL_FULLSCREEN:0));
      if (display == NULL) {
          fprintf(stderr, "SDL_SetVideoMode error: %s\n",
                  SDL_GetError());
          exit(1);
      }
      if ((w == 640) && (h == 480)) {
          loop->setSurface(display);
          loop->setDisplay(NULL);
      }
      else {
          SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 32,0,0,0,0);
          loop->setSurface(surface);
          loop->setDisplay(display);
      }
      loop->setOpenGLMode(false);
  }
  atexit(SDL_Quit);
  SDL_ShowCursor(SDL_DISABLE);
  SDL_WM_SetCaption("FloboPuyo by iOS-Software",NULL);
  /* Should also set up an icon someday */
  //SDL_WM_SetIcon(SDL_LoadBMP("icon.bmp"), NULL);
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


