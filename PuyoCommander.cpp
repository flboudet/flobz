/* strings to translate */

#include "PuyoCommander.h"
#include "PuyoStrings.h"
#include "preferences.h"
#include "audio.h"
#include "MainMenu.h"

using namespace gameui;
using namespace event_manager;

PuyoCommander *theCommander = NULL;
IosFont *storyFont;

static const char * kFullScreenPref = "Config.FullScreen";
#ifdef HAVE_OPENGL
static const char * kOpenGLPref     = "Config.OpenGL";
#endif

IosSurface *IosSurfaceFactory::create(const char *resourcePath)
{
    ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
    return iimLib.loadImage(IMAGE_RGB, m_dataPathManager.getPath(resourcePath));
}

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

PuyoCommander::PuyoCommander(String dataDir, int maxDataPackNumber)
  : dataPathManager(dataDir), m_surfaceFactory(dataPathManager), m_cursor(NULL)
{
  loop = GameUIDefaults::GAME_LOOP;
  mbox = NULL;
  theCommander = this;

  if (maxDataPackNumber != -1)
      dataPathManager.setMaxPackNumber(maxDataPackNumber);
  createResourceManagers();
}

void PuyoCommander::initWithGUI(bool fs)
{
  m_windowFramePicture = std::auto_ptr<FramePicture>(new FramePicture(25, 28, 25, 19, 26, 23));
  m_buttonIdleFramePicture = std::auto_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  m_buttonDownFramePicture = std::auto_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  m_buttonOverFramePicture = std::auto_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  m_textFieldIdleFramePicture = std::auto_ptr<FramePicture>(new FramePicture(5, 23, 4, 6, 10, 3));
  m_separatorFramePicture = std::auto_ptr<FramePicture>(new FramePicture(63, 2, 63, 2, 4, 2));
  m_listFramePicture = std::auto_ptr<FramePicture>(new FramePicture(5, 23, 4, 6, 10, 3));

  loadPreferences(fs);

  initLocale();
  initAudio();
  initFonts();

  // Loading the frame images, and setting up the frames
  ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
  m_switchOnImage = iimLib.loadImage(IMAGE_RGBA, dataPathManager.getPath("gfx/switch-on.png"));
  m_switchOffImage = iimLib.loadImage(IMAGE_RGBA, dataPathManager.getPath("gfx/switch-off.png"));
  m_radioOnImage = iimLib.loadImage(IMAGE_RGBA, dataPathManager.getPath("gfx/radio-on.png"));
  m_radioOffImage = iimLib.loadImage(IMAGE_RGBA, dataPathManager.getPath("gfx/radio-off.png"));
  m_upArrow = iimLib.loadImage(IMAGE_RGBA, theCommander->getDataPathManager().getPath("gfx/uparrow.png"));
  m_downArrow = iimLib.loadImage(IMAGE_RGBA, theCommander->getDataPathManager().getPath("gfx/downarrow.png"));
  m_leftArrow = iimLib.loadImage(IMAGE_RGBA, theCommander->getDataPathManager().getPath("gfx/leftarrow.png"));
  m_rightArrow = iimLib.loadImage(IMAGE_RGBA, theCommander->getDataPathManager().getPath("gfx/rightarrow.png"));

  m_frameImage = m_surfaceResManager->getResource("gfx/frame.png");
  m_buttonIdleImage = iimLib.loadImage(IMAGE_RGB, dataPathManager.getPath("gfx/button.png"));
  m_buttonDownImage = iimLib.loadImage(IMAGE_RGB, dataPathManager.getPath("gfx/buttondown.png"));
  m_buttonOverImage = iimLib.loadImage(IMAGE_RGB, dataPathManager.getPath("gfx/buttonover.png"));
  m_textFieldIdleImage = iimLib.loadImage(IMAGE_RGB, dataPathManager.getPath("gfx/editfield.png"));
  m_separatorImage = iimLib.loadImage(IMAGE_RGB, dataPathManager.getPath("gfx/separator.png"));
  m_listIdleImage = iimLib.loadImage(IMAGE_RGB, dataPathManager.getPath("gfx/listborder.png"));

  m_windowFramePicture->setFrameSurface(m_frameImage);
  m_buttonIdleFramePicture->setFrameSurface(m_buttonIdleImage);
  m_buttonDownFramePicture->setFrameSurface(m_buttonDownImage);
  m_buttonOverFramePicture->setFrameSurface(m_buttonOverImage);
  m_textFieldIdleFramePicture->setFrameSurface(m_textFieldIdleImage);
  m_separatorFramePicture->setFrameSurface(m_separatorImage);
  m_listFramePicture->setFrameSurface(m_listIdleImage);
}

void PuyoCommander::initWithoutGUI()
{
  initLocale();
}

PuyoCommander::~PuyoCommander()
{
  //delete m_frameImage;
  delete m_buttonIdleImage;
  delete m_buttonDownImage;
  delete m_buttonOverImage;
  delete m_textFieldIdleImage;
  delete m_separatorImage;
  delete m_slideSound;
  delete m_whipSound;
  delete m_whopSound;
}

extern char *dataFolder;
#include <string>

/* Initialise the default dictionnary */
void PuyoCommander::initLocale()
{
  locale = new LocalizedDictionary(dataPathManager, "locale", "main");
}

/* Global translator */
const char * PuyoCommander::getLocalizedString(const char * originalString) const
{
  return locale->getLocalizedString(originalString);
}

/* Initialize the audio if necessary */
void PuyoCommander::initAudio()
{
    m_slideSound = loop->getAudioManager()->loadSound(dataPathManager.getPath(FilePath("sfx").combine("slide.wav")));
    m_whipSound = loop->getAudioManager()->loadSound(dataPathManager.getPath(FilePath("sfx").combine("whip.wav")));
    m_whopSound = loop->getAudioManager()->loadSound(dataPathManager.getPath(FilePath("sfx").combine("whop.wav")));
    GameUIDefaults::SLIDE_SOUND = m_slideSound;
    AudioManager::init();
}


/* load fonts and set them for use in the GUI */
void PuyoCommander::initFonts()
{
  DBG_PRINT("initFonts()\n");

  IosFont * darkFont;
  IosFont * menuFont;
  IosFont * smallFont;
  IosFont * smallFontInfo;
  IosFont * textFont;
  IosFont * funnyFont;

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

  ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
  darkFont = iimLib.createFont(font, 17, Font_DARK);
  menuFont = iimLib.createFont(font, 17, Font_STD);
  smallFont = iimLib.createFont(font, 12, Font_STD);
  smallFontInfo = iimLib.createFont(font, 12, Font_DARK);
  textFont = iimLib.createFont(font, 17, Font_GREY);
  funnyFont = iimLib.createFont(funny_path, 24, Font_STD);
  storyFont = iimLib.createFont(font, 17, Font_STORY);

  GameUIDefaults::FONT              = menuFont;
  GameUIDefaults::FONT_TEXT         = textFont;
  GameUIDefaults::FONT_INACTIVE     = darkFont;
  GameUIDefaults::FONT_SMALL_INFO   = smallFontInfo;
  GameUIDefaults::FONT_SMALL_ACTIVE = smallFont;
  GameUIDefaults::FONT_FUNNY        = funnyFont;
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

void PuyoCommander::setGlSDL(bool useGL)
{
}

/* load a few important preferences for display */
void PuyoCommander::loadPreferences(bool fs)
{
  DBG_PRINT("loadPreferences()\n");
  /* Load Preferences */
  //fullscreen = fs ? GetBoolPreference(kFullScreenPref, true) : false;
#ifdef HAVE_OPENGL
  useGL = GetBoolPreference(kOpenGLPref, false);
#else
  useGL = false;
#endif
}

ScreenTransitionWidget *PuyoCommander::createScreenTransition(Screen &fromScreen) const
{
    return new DoomMeltScreenTransitionWidget(fromScreen);
}

void PuyoCommander::registerCursor(AbstractCursor *cursor)
{
    m_cursor = cursor;
}

void PuyoCommander::setCursorVisible(bool visible)
{
    if (m_cursor != NULL)
        m_cursor->setVisible(visible);
}

void PuyoCommander::createResourceManagers()
{
    m_surfaceResManager.reset(new SimpleResourceManager<IosSurface>(m_surfaceFactory));
}
