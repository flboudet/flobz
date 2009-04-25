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

  initLocale();
  initGameControls();
  initAudio();
  initFonts();

    // Loading the frame images, and setting up the frames
    IIMLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getIIMLibrary();
    m_switchOnImage = iimLib.load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/switch-on.png"));
    m_switchOffImage = iimLib.load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/switch-off.png"));
    m_radioOnImage = iimLib.load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/radio-on.png"));
    m_radioOffImage = iimLib.load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/radio-off.png"));
    m_upArrow = iimLib.load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/uparrow.png"));
    m_downArrow = iimLib.load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/downarrow.png"));
    m_leftArrow = iimLib.load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/leftarrow.png"));
    m_rightArrow = iimLib.load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/rightarrow.png"));

    m_frameImage = iimLib.load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/frame.png"));
    m_buttonIdleImage = iimLib.load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/button.png"));
    m_buttonDownImage = iimLib.load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/buttondown.png"));
    m_buttonOverImage = iimLib.load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/buttonover.png"));
    m_textFieldIdleImage = iimLib.load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/editfield.png"));
    m_separatorImage = iimLib.load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/separator.png"));
    m_listIdleImage = iimLib.load_Absolute_DisplayFormatAlpha(dataPathManager.getPath("gfx/listborder.png"));
    m_windowFramePicture.setFrameSurface(m_frameImage);
    m_buttonIdleFramePicture.setFrameSurface(m_buttonIdleImage);
    m_buttonDownFramePicture.setFrameSurface(m_buttonDownImage);
    m_buttonOverFramePicture.setFrameSurface(m_buttonOverImage);
    m_textFieldIdleFramePicture.setFrameSurface(m_textFieldIdleImage);
    m_separatorFramePicture.setFrameSurface(m_separatorImage);
    m_listFramePicture.setFrameSurface(m_listIdleImage);
}

PuyoCommander::~PuyoCommander()
{
  delete m_frameImage;
  delete m_buttonIdleImage;
  delete m_buttonDownImage;
  delete m_buttonOverImage;
  delete m_textFieldIdleImage;
  delete m_separatorImage;
  gameui::GlobalNotificationCenter.removeListener(getFullScreenKey(),this);
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
#ifdef DISABLED
    if (fullScreen != this->fullscreen) {
        this->fullscreen = fullScreen;
        SetBoolPreference(kFullScreenPref, fullscreen);
        if (SDL_WM_ToggleFullScreen(loop->getSurface())==0)
        {
          // This should not be necessary (and actually prevents Windows
          // going from fullscreen to windowed mode)
          //SDL_QuitSubSystem(SDL_INIT_VIDEO);
          //SDL_InitSubSystem(SDL_INIT_VIDEO);

            //initDisplay(GetIntPreference(kScreenWidthPref, 640),
            //        GetIntPreference(kScreenHeightPref, 480), fullscreen, useGL);
        }
        /* Workaround for cursor showing in MacOS X fullscreen mode */
        SDL_ShowCursor(SDL_ENABLE);
        SDL_ShowCursor(SDL_DISABLE);
    }
#endif
}

void PuyoCommander::setGlSDL(bool useGL)
{
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

