/* strings to translate */

#include "PuyoCommander.h"
#include "PuyoStrings.h"
#include "preferences.h"
#include "audio.h"
#include "MainMenu.h"
#ifdef THREADED_RESOURCE_MANAGER
#include "ThreadedResourceManager.h"
#endif
using namespace gameui;
using namespace event_manager;

PuyoCommander *theCommander = NULL;
IosFont *storyFont;

static const char * kFullScreenPref = "Config.FullScreen";
#ifdef HAVE_OPENGL
static const char * kOpenGLPref     = "Config.OpenGL";
#endif

#ifdef PRODUCE_CACHE_FILE
FILE *cacheOutputGsl;
#endif

IosSurface *IosSurfaceFactory::create(const IosSurfaceResourceKey &resourceKey)
{
    try {
#ifdef PRODUCE_CACHE_FILE
        fprintf(cacheOutputGsl, "  [cache_picture: path=\"%s\" mode=%d]\n",
                resourceKey.path.c_str(), resourceKey.type);
#endif
        String fullPath = m_dataPathManager.getPath(resourceKey.path.c_str());
        ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
        IosSurface *newSurface = iimLib.loadImage(resourceKey.type, fullPath, resourceKey.specialAbility);
        newSurface->enableExceptionOnDeletion(true);
        return newSurface;
    }
    catch (Exception e) {
        return NULL;
    }
}

void IosSurfaceFactory::destroy(IosSurface *res)
{
    res->enableExceptionOnDeletion(false);
    delete res;
}

audio_manager::Sound * SoundFactory::create(const std::string &path)
{
    try {
#ifdef PRODUCE_CACHE_FILE
        fprintf(cacheOutputGsl, "  [cache_sound: path=\"%s\"]\n", path.c_str());
#endif
        String fullPath = m_dataPathManager.getPath(path.c_str());
        audio_manager::Sound *newSound = GameUIDefaults::GAME_LOOP->getAudioManager()->loadSound(fullPath);
        return newSound;
    }
    catch (Exception e) {
        return NULL;
    }
}

void SoundFactory::destroy(audio_manager::Sound *res)
{
    delete res;
}

audio_manager::Music * MusicFactory::create(const std::string &path)
{
    try {
#ifdef PRODUCE_CACHE_FILE
        fprintf(cacheOutputGsl, "  [cache_music: path=\"%s\"]\n", path.c_str());
#endif
        String fullPath = m_dataPathManager.getPath(path.c_str());
        audio_manager::Music *newMusic = GameUIDefaults::GAME_LOOP->getAudioManager()->loadMusic(fullPath);
        return newMusic;
    }
    catch (Exception e) {
        return NULL;
    }
}

void MusicFactory::destroy(audio_manager::Music *res)
{
    delete res;
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
  : dataPathManager(dataDir),
    m_surfaceFactory(dataPathManager),
    m_soundFactory(dataPathManager),
    m_musicFactory(dataPathManager),
    m_cursor(NULL)
{
#ifdef PRODUCE_CACHE_FILE
  cacheOutputGsl = fopen("cache.gsl", "w");
#endif
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
  m_switchOnImage = getSurface(IMAGE_RGBA, "gfx/switch-on.png");
  m_switchOffImage = getSurface(IMAGE_RGBA, "gfx/switch-off.png");
  m_radioOnImage = getSurface(IMAGE_RGBA, "gfx/radio-on.png");
  m_radioOffImage = getSurface(IMAGE_RGBA, "gfx/radio-off.png");
  m_upArrow = getSurface(IMAGE_RGBA, "gfx/uparrow.png");
  m_downArrow = getSurface(IMAGE_RGBA, "gfx/downarrow.png");
  m_leftArrow = getSurface(IMAGE_RGBA, "gfx/leftarrow.png");
  m_rightArrow = getSurface(IMAGE_RGBA, "gfx/rightarrow.png");

  m_frameImage = getSurface(IMAGE_RGBA, "gfx/frame.png");
  m_buttonIdleImage = getSurface(IMAGE_RGBA, "gfx/button.png");
  m_buttonDownImage = getSurface(IMAGE_RGBA, "gfx/buttondown.png");
  m_buttonOverImage = getSurface(IMAGE_RGBA, "gfx/buttonover.png");
  m_textFieldIdleImage = getSurface(IMAGE_RGBA, "gfx/editfield.png");
  m_separatorImage = getSurface(IMAGE_RGBA, "gfx/separator.png");
  m_listIdleImage = getSurface(IMAGE_RGBA, "gfx/listborder.png");

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
    m_slideSound = getSound(FilePath("sfx").combine("slide.wav"));
    m_whipSound = getSound(FilePath("sfx").combine("whip.wav"));
    m_whopSound = getSound(FilePath("sfx").combine("whop.wav"));
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

// Resource management
void PuyoCommander::cacheSurface(ImageType type, const char *path, ImageSpecialAbility specialAbility)
{
    m_surfaceResManager->cacheResource(IosSurfaceResourceKey(type, path, specialAbility));
}

IosSurfaceRef PuyoCommander::getSurface(ImageType type, const char *path, ImageSpecialAbility specialAbility)
{
    return m_surfaceResManager->getResource(IosSurfaceResourceKey(type, path, specialAbility));
}

void PuyoCommander::cacheSound(const char *path)
{
    m_soundResManager->cacheResource(path);
}

SoundRef PuyoCommander::getSound(const char *path)
{
    return m_soundResManager->getResource(path);
}

void PuyoCommander::cacheMusic(const char *path)
{
    m_musicResManager->cacheResource(path);
}

MusicRef PuyoCommander::getMusic(const char *path)
{
    return m_musicResManager->getResource(path);
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
#ifdef THREADED_RESOURCE_MANAGER
    m_surfaceResManager.reset(new ThreadedResourceManager<IosSurface, IosSurfaceResourceKey>(m_surfaceFactory));
    m_soundResManager.reset(new ThreadedResourceManager<audio_manager::Sound>(m_soundFactory));
    m_musicResManager.reset(new ThreadedResourceManager<audio_manager::Music>(m_musicFactory));
#else
    m_surfaceResManager.reset(new SimpleResourceManager<IosSurface, IosSurfaceResourceKey>(m_surfaceFactory));
    m_soundResManager.reset(new SimpleResourceManager<audio_manager::Sound>(m_soundFactory));
    m_musicResManager.reset(new SimpleResourceManager<audio_manager::Music>(m_musicFactory));
#endif
}
