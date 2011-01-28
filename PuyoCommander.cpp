/* strings to translate */

#include "GTLog.h"
#include "PuyoCommander.h"
#include "PuyoStrings.h"
#include "preferences.h"
#include "audio.h"
#include "MainMenu.h"
#include "AnimatedPuyoTheme.h"
#ifdef THREADED_RESOURCE_MANAGER
#include "ThreadedResourceManager.h"
#endif
using namespace gameui;
using namespace event_manager;

PuyoCommander *theCommander = NULL;
IosFont *storyFont; // TODO: remove

static const char * kFullScreenPref = "Config.FullScreen";

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
        ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
        IosSurface *newSurface = iimLib.loadImage(resourceKey.type, resourceKey.path.c_str(), resourceKey.specialAbility);
        if (newSurface != NULL)
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

IosFont *IosFontFactory::create(const IosFontResourceKey &resourceKey)
{
    try {
#ifdef PRODUCE_CACHE_FILE
        fprintf(cacheOutputGsl, "  [cache_picture: path=\"%s\" mode=%d]\n",
                resourceKey.path.c_str(), resourceKey.type);
#endif
        String fullPath = m_dataPathManager.getPath(resourceKey.path.c_str());
        ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
        IosFont *newFont = iimLib.createFont(fullPath, resourceKey.size, resourceKey.fx);
        return newFont;
    }
    catch (Exception e) {
        return NULL;
    }
}

void IosFontFactory::destroy(IosFont *res)
{
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

PuyoSetThemeFactory::PuyoSetThemeFactory(ThemeManager &themeManager)
    : m_themeManager(themeManager)
{
}

PuyoSetTheme *PuyoSetThemeFactory::create(const std::string &name)
{
    return m_themeManager.createPuyoSetTheme(name);
}

void PuyoSetThemeFactory::destroy(PuyoSetTheme *res)
{
    delete res;
}

LevelThemeFactory::LevelThemeFactory(ThemeManager &themeManager)
    : m_themeManager(themeManager)
{
}

LevelTheme *LevelThemeFactory::create(const std::string &name)
{
    return m_themeManager.createLevelTheme(name);
}

void LevelThemeFactory::destroy(LevelTheme *res)
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

PuyoCommander::PuyoCommander(DataPathManager &dataPathManager)
  : dataPathManager(dataPathManager),
    m_themeManager(new ThemeManagerImpl(dataPathManager)),
    m_surfaceFactory(dataPathManager),
    m_fontFactory(dataPathManager),
    m_soundFactory(dataPathManager),
    m_musicFactory(dataPathManager),
    m_puyoSetThemeFactory(*m_themeManager),
    m_levelThemeFactory(*m_themeManager),
    m_cursor(NULL)
{
#ifdef PRODUCE_CACHE_FILE
  cacheOutputGsl = fopen("cache.gsl", "w");
#endif
  loop = GameUIDefaults::GAME_LOOP;
  theCommander = this;


  createResourceManagers();
}

void PuyoCommander::initWithGUI(bool fs)
{
  GTLogTrace("PuyoCommander::initWithGUI() entered");

  m_windowFramePicture = std::auto_ptr<FramePicture>(new FramePicture(25, 28, 25, 19, 26, 23));
  m_buttonIdleFramePicture = std::auto_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  m_buttonDownFramePicture = std::auto_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  m_buttonSpecialFramePicture = std::auto_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  m_buttonOverFramePicture = std::auto_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  m_textFieldIdleFramePicture = std::auto_ptr<FramePicture>(new FramePicture(5, 23, 4, 6, 10, 3));
  m_separatorFramePicture = std::auto_ptr<FramePicture>(new FramePicture(63, 2, 63, 2, 4, 2));
  m_listFramePicture = std::auto_ptr<FramePicture>(new FramePicture(5, 23, 4, 6, 10, 3));

  GTLogTrace("PuyoCommander::initWithGUI() loading prefs");

  loadPreferences(fs);

  GTLogTrace("PuyoCommander::initWithGUI() init locales");

  initLocale();

  GTLogTrace("PuyoCommander::initWithGUI() init audio");

  initAudio();

  GTLogTrace("PuyoCommander::initWithGUI() init fonts");

  initFonts();

  GTLogTrace("PuyoCommander::initWithGUI() loading images");


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
  m_buttonSpecialImage = getSurface(IMAGE_RGBA, "gfx/buttonspecial.png");
  m_textFieldIdleImage = getSurface(IMAGE_RGBA, "gfx/editfield.png");
  m_separatorImage = getSurface(IMAGE_RGBA, "gfx/separator.png");
  m_listIdleImage = getSurface(IMAGE_RGBA, "gfx/listborder.png");

  GTLogTrace("PuyoCommander::initWithGUI() configuring frames");

  m_windowFramePicture->setFrameSurface(m_frameImage);
  m_buttonIdleFramePicture->setFrameSurface(m_buttonIdleImage);
  m_buttonSpecialFramePicture->setFrameSurface(m_buttonSpecialImage);
  m_buttonDownFramePicture->setFrameSurface(m_buttonDownImage);
  m_buttonOverFramePicture->setFrameSurface(m_buttonOverImage);
  m_textFieldIdleFramePicture->setFrameSurface(m_textFieldIdleImage);
  m_separatorFramePicture->setFrameSurface(m_separatorImage);
  m_listFramePicture->setFrameSurface(m_listIdleImage);
  GTLogTrace("PuyoCommander::initWithGUI() completed");

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
    Locales_Init(); // Make sure locales are detected.
    String fontName, funnyFontName;
    fontName = locale->getLocalizedString("__FONT__");
    m_localizedFontName = fontName;
    funnyFontName = "gfx/zill_spills.ttf";
    /*try {
     font = dataPathManager.getPath(locale->getLocalizedString("__FONT__"));
     }
     catch (ios_fc::Exception) {
     fprintf(stderr,"Font %s not found.\n", (const char *)font);
     font = dataPathManager.getPath("gfx/font.ttf");
     fprintf(stderr,"Using default font %s.\n", (const char *)font);
     }
     funny_path = getDataPathManager().getPath("gfx/zill_spills.ttf");*/

    m_darkFont = getFont(fontName, 17, Font_DARK);
    m_menuFont = getFont(fontName, 17, Font_STD);
    m_smallFont = getFont(fontName, 12, Font_STD);
    m_smallFontInfo = getFont(fontName, 12, Font_DARK);
    m_textFont = getFont(fontName, 17, Font_GREY);
    m_funnyFont = getFont(funnyFontName, 24, Font_STD);
    storyFont = getFont(fontName, 17, Font_STORY);

    GameUIDefaults::FONT              = m_menuFont;
    GameUIDefaults::FONT_TEXT         = m_textFont;
    GameUIDefaults::FONT_INACTIVE     = m_darkFont;
    GameUIDefaults::FONT_SMALL_INFO   = m_smallFontInfo;
    GameUIDefaults::FONT_SMALL_ACTIVE = m_smallFont;
    GameUIDefaults::FONT_FUNNY        = m_funnyFont;
}

void PuyoCommander::initThemes()
{
#ifdef DISABLED
    // List the themes in the various pack folders
    SelfVector<String> themeFolders = getDataPathManager().getEntriesAtPath("theme");
    // Load the themes from the list (only those matching the correct extension)
    for (int i = 0 ; i < themeFolders.size() ; i++) {
        if (themeFolders[i].substring(themeFolders[i].size() - 8)
            == themeFolderExtension) {
            cout << "Theme to be loaded: " << (const char *)(themeFolders[i]); << endl;
            //loadTheme(themeFolders[i]);
        }
    }
#endif
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

/* load a few important preferences for display */
void PuyoCommander::loadPreferences(bool fs)
{
  DBG_PRINT("loadPreferences()\n");
  /* Load Preferences */
  //fullscreen = fs ? GetBoolPreference(kFullScreenPref, true) : false;
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

IosSurfaceRef PuyoCommander::getSurface(ImageType type, const char *path, const ImageOperationList &list)
{
    ImageSpecialAbility specialAbility = GameUIDefaults::GAME_LOOP->getDrawContext()->guessRequiredImageAbility(list);
    return m_surfaceResManager->getResource(IosSurfaceResourceKey(type, path, specialAbility));
}

void PuyoCommander::cacheFont(const char *path, int size, IosFontFx fx)
{
    m_fontResManager->cacheResource(IosFontResourceKey(path, size, fx));
}

IosFontRef PuyoCommander::getFont(const char *path, int size, IosFontFx fx)
{
    return m_fontResManager->getResource(IosFontResourceKey(path, size, fx));
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

PuyoSetThemeRef PuyoCommander::getPuyoSetTheme(const char *name)
{
    return m_puyoSetThemeResManager->getResource(name);
}

PuyoSetThemeRef PuyoCommander::getPreferedPuyoSetTheme()
{
    return getPuyoSetTheme(getPreferedPuyoSetThemeName().c_str());
}

const std::string &PuyoCommander::getPreferedPuyoSetThemeName() const
{
    if (m_defaultPuyoSetThemeName == "") {
        char out[256];
        GetStrPreference ("puyoset_theme", out, getDefaultPuyoSetThemeName().c_str());
        m_defaultPuyoSetThemeName = out;
    }
    return m_defaultPuyoSetThemeName;
}

const std::string PuyoCommander::getDefaultPuyoSetThemeName() const
{
    // TODO: provide a way to set the default theme in the data
    //return m_themeManager->getPuyoSetThemeList()[0];
    return m_themeManager->getPuyoSetThemeList()[m_themeManager->getPuyoSetThemeList().size() - 1];
}


void PuyoCommander::setPreferedPuyoSetThemeName(const char *name)
{
    m_defaultPuyoSetThemeName = name;
    SetStrPreference ("puyoset_theme", name);
}

const std::vector<std::string> &PuyoCommander::getPuyoSetThemeList() const
{
    return m_themeManager->getPuyoSetThemeList();
}

LevelThemeRef PuyoCommander::getLevelTheme(const char *name)
{
    return m_levelThemeResManager->getResource(name);
}

LevelThemeRef PuyoCommander::getPreferedLevelTheme()
{
    return getLevelTheme(getPreferedLevelThemeName().c_str());
}

const std::string &PuyoCommander::getPreferedLevelThemeName() const
{
    if (m_defaultLevelThemeName == "") {
        char out[256];
        GetStrPreference ("level_theme", out, getDefaultLevelThemeName().c_str());
        m_defaultLevelThemeName = out;
    }
    return m_defaultLevelThemeName;
}

const std::string PuyoCommander::getDefaultLevelThemeName() const
{
    return m_themeManager->getLevelThemeList()[0];
}

void PuyoCommander::setPreferedLevelThemeName(const char *name)
{
    m_defaultLevelThemeName = name;
    SetStrPreference ("level_theme", name);
}

const std::vector<std::string> &PuyoCommander::getLevelThemeList() const
{
    return m_themeManager->getLevelThemeList();
}

void PuyoCommander::freeUnusedResources()
{
    m_surfaceResManager->freeUnusedResources();
    //m_fontResManager->freeUnusedResources();
    m_soundResManager->freeUnusedResources();
    //m_musicResManager->freeUnusedResources();
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
    m_fontResManager.reset(new ThreadedResourceManager<IosFont, IosFontResourceKey>(m_fontFactory));
    m_soundResManager.reset(new ThreadedResourceManager<audio_manager::Sound>(m_soundFactory));
    m_musicResManager.reset(new ThreadedResourceManager<audio_manager::Music>(m_musicFactory));
#else
    m_surfaceResManager.reset(new SimpleResourceManager<IosSurface, IosSurfaceResourceKey>(m_surfaceFactory));
    m_fontResManager.reset(new SimpleResourceManager<IosFont, IosFontResourceKey>(m_fontFactory));
    m_soundResManager.reset(new SimpleResourceManager<audio_manager::Sound>(m_soundFactory));
    m_musicResManager.reset(new SimpleResourceManager<audio_manager::Music>(m_musicFactory));
    m_puyoSetThemeResManager.reset(new SimpleResourceManager<PuyoSetTheme>(m_puyoSetThemeFactory));
    m_levelThemeResManager.reset(new SimpleResourceManager<LevelTheme>(m_levelThemeFactory));
#endif
}
