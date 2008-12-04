/* strings to translate */

#include "PuyoCommander.h"
#include "PuyoStrings.h"
#include "preferences.h"
#include "audio.h"
#include "MainMenu.h"
#include "PuyoSinglePlayerStarter.h"

#include "PuyoNetworkStarter.h"

using namespace gameui;

PuyoCommander *theCommander = NULL;
SoFont *storyFont;

static const char * kFullScreenPref = "Config.FullScreen";
#ifdef HAVE_OPENGL
static const char * kOpenGLPref     = "Config.OpenGL";
#endif
static const char * kScreenWidthPref = "Config.ScreenWidth";
static const char * kScreenHeightPref = "Config.ScreenHeight";



/*
 * THE MENUS
 */

/**
 * Launches a single player game
 */
void SinglePlayerGameAction::action()
{
  /*PuyoStarter *starter = new PuyoSinglePlayerStarter(theCommander, 5, FLOBO, 0);
  starter->run(0,0,0,0,0);
  GameUIDefaults::SCREEN_STACK->push(starter);*/
}

void PuyoCommander::debug_gsl(String gsl_script)
{
  GameUIDefaults::SCREEN_STACK->push(mainScreen);
  StoryScreen story_screen(gsl_script);
  GameUIDefaults::SCREEN_STACK->push(&story_screen);
  GameUIDefaults::GAME_LOOP->run();
}

void PuyoCommander::run()
{
  GameUIDefaults::SCREEN_STACK->push(mainScreen);
  GameUIDefaults::GAME_LOOP->run();
}

void PuyoCommander::initMenus()
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

/* Build the PuyoCommander */

PuyoCommander::PuyoCommander(String dataDir, bool fs, int maxDataPackNumber)
  : dataPathManager(dataDir),
    m_windowFramePicture(25, 28, 25, 19, 26, 23),
    m_buttonIdleFramePicture(13, 10, 13, 12, 7, 13),
    m_buttonDownFramePicture(13, 10, 13, 12, 7, 13),
    m_buttonOverFramePicture(13, 10, 13, 12, 7, 13),
    m_textFieldIdleFramePicture(5, 23, 4, 6, 10, 3),
    m_separatorFramePicture(63, 2, 63, 2, 4, 2),
    m_listFramePicture(5, 23, 4, 6, 10, 3)
{
  //SDL_Delay(500);
  loop = GameUIDefaults::GAME_LOOP;
  mbox = NULL;
  theCommander = this;

  if (maxDataPackNumber != -1)
    dataPathManager.setMaxPackNumber(maxDataPackNumber);
  loadPreferences(fs);
  gameui::GlobalNotificationCenter.addListener(getFullScreenKey(),this);
  initSDL();
  initLocale();
  initGameControls();
  initAudio();
  initDisplay(GetIntPreference(kScreenWidthPref, 640),
          GetIntPreference(kScreenHeightPref, 480), fullscreen, useGL);
  initFonts();

    // Loading the frame images, and setting up the frames
    m_switchOnImage = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/switch-on.png"));
    m_switchOffImage = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/switch-off.png"));
    m_radioOnImage = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/radio-on.png"));
    m_radioOffImage = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/radio-off.png"));
    m_frameImage = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/frame.png"));
    m_buttonIdleImage = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/button.png"));
    m_buttonDownImage = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/buttondown.png"));
    m_buttonOverImage = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/buttonover.png"));
    m_textFieldIdleImage = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/editfield.png"));
    m_separatorImage = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/separator.png"));
    m_listIdleImage = IIM_Load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/listborder.png"));
    m_windowFramePicture.setFrameSurface(m_frameImage);
    m_buttonIdleFramePicture.setFrameSurface(m_buttonIdleImage);
    m_buttonDownFramePicture.setFrameSurface(m_buttonDownImage);
    m_buttonOverFramePicture.setFrameSurface(m_buttonOverImage);
    m_textFieldIdleFramePicture.setFrameSurface(m_textFieldIdleImage);
    m_separatorFramePicture.setFrameSurface(m_separatorImage);
    m_listFramePicture.setFrameSurface(m_listIdleImage);

  initMenus();
  cursor = new GameCursor(dataPathManager.getPath("gfx/cursor.png"));
  loop->addDrawable(cursor);
  loop->addIdle(cursor);

}

PuyoCommander::~PuyoCommander()
{
  IIM_Free(m_frameImage);
  IIM_Free(m_buttonIdleImage);
  IIM_Free(m_buttonDownImage);
  IIM_Free(m_buttonOverImage);
  IIM_Free(m_textFieldIdleImage);
  IIM_Free(m_separatorImage);
  gameui::GlobalNotificationCenter.removeListener(getFullScreenKey(),this);
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

/* Initialise the default dictionnary */
void PuyoCommander::initLocale()
{
  locale = new PuyoLocalizedDictionary(dataPathManager, "locale", "main");
}

/* Global translator */
const char * PuyoCommander::getLocalizedString(const char * originalString) const
{
  return locale->getLocalizedString(originalString);
}

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

  SoFont * darkFont = SoFont_new();
  SoFont * menuFont = SoFont_new();
  SoFont * smallFont = SoFont_new();
  SoFont * smallFontInfo = SoFont_new();
  SoFont * textFont = SoFont_new();
  SoFont * funnyFont = SoFont_new();

#ifdef ENABLE_TTF
  Locales_Init(); // Make sure locales are detected.
  String font, funny_path;
  try {
    font = dataPathManager.getPath(locale->getLocalizedString("__FONT__"));
  }
  catch (ios_fc::Exception) {
    fprintf(stderr,"Font %s not found.\n", (const char *)font);
    font = dataPathManager.getPath("gfx/font.ttf");
    fprintf(stderr,"Using default font %s.\n", (const char *)font);
  }
  funny_path = getDataPathManager().getPath("gfx/zill_spills.ttf");

  SoFont_load_ttf(darkFont, font, 17, SoFont_DARK);
  SoFont_load_ttf(menuFont, font, 17, SoFont_STD);
  SoFont_load_ttf(smallFont, font, 12, SoFont_STD);
  SoFont_load_ttf(smallFontInfo, font, 12, SoFont_DARK);
  SoFont_load_ttf(textFont, font, 17, SoFont_GREY);
  SoFont_load_ttf(funnyFont, funny_path, 24, SoFont_STD);

  storyFont = SoFont_new();
  SoFont_load_ttf(storyFont, font, 17, SoFont_STORY);
#else
  smallFontInfo = SoFont_new();
  IIM_Surface *font3b = IIM_Load_Absolute_DisplayFormatAlpha (dataPathManager.getPath("gfx/font3b.png"));
  IIM_Surface *font4b = IIM_Load_Absolute_DisplayFormatAlpha (dataPathManager.getPath("gfx/font4b.png"));
  IIM_Surface *font6b = iim_surface_shift_hsv(font4b, 170, -.5, -.2);
  IIM_Surface *font5b = iim_surface_shift_hsv(font3b, 170, -.5, -.2);

  SoFont_load(darkFont, IIM_Load_Absolute_DisplayFormatAlpha (dataPathManager.getPath("gfx/fontdark.png")));
  SoFont_load(menuFont, font3b);
  SoFont_load(smallFont, font4b);
  SoFont_load(smallFontInfo, font6b);
  SoFont_load(textFont, font5b);
  funnyFont = menuFont;

  storyFont = darkFont;
#endif

  SoFont_free(GameUIDefaults::FONT);
  GameUIDefaults::FONT              = menuFont;
  SoFont_free(GameUIDefaults::FONT_TEXT);
  GameUIDefaults::FONT_TEXT         = textFont;
  SoFont_free(GameUIDefaults::FONT_INACTIVE);
  GameUIDefaults::FONT_INACTIVE     = darkFont;
  SoFont_free(GameUIDefaults::FONT_SMALL_INFO);
  GameUIDefaults::FONT_SMALL_INFO   = smallFontInfo;
  SoFont_free(GameUIDefaults::FONT_SMALL_ACTIVE);
  GameUIDefaults::FONT_SMALL_ACTIVE = smallFont;
  SoFont_free(GameUIDefaults::FONT_FUNNY);
  GameUIDefaults::FONT_FUNNY        = funnyFont;
}


void PuyoCommander::notificationOccured(String identifier, void * context)
{
    if (identifier == kFullScreenPref) {
        setFullScreen(*(bool *)context);
    }
}
/*
void PuyoCommander::setMusic(bool music)
{
  AudioManager::musicOnOff(music);
}

void PuyoCommander::setSoundFx(bool fx)
{
  AudioManager::soundOnOff(fx);
}
*/
bool PuyoCommander::getMusic()
{
  return AudioManager::isMusicOn();
}

bool PuyoCommander::getSoundFx()
{
  return AudioManager::isSoundOn();
}

String PuyoCommander::getFullScreenKey(void) const
{
    return String(kFullScreenPref);
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

            initDisplay(GetIntPreference(kScreenWidthPref, 640),
                    GetIntPreference(kScreenHeightPref, 480), fullscreen, useGL);
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
void PuyoCommander::initDisplay(int w, int h, bool fullscreen, bool useGL)
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


/* load a few important preferences for display */
void PuyoCommander::loadPreferences(bool fs)
{
  DBG_PRINT("loadPreferences()\n");
  /* Load Preferences */
  fullscreen = fs ? GetBoolPreference(kFullScreenPref, true) : false;
#ifdef HAVE_OPENGL
  useGL = GetBoolPreference(kOpenGLPref, false);
#else
  useGL = false;
#endif
}

void PuyoCommander::onMessage(Message &msb) {}
