/* strings to translate */

#include <sstream>
#include "GTLog.h"
#include "FPCommander.h"
#include "FPStrings.h"
#include "audio.h"
#include "MainMenu.h"
#include "AnimatedFloboTheme.h"
#ifdef THREADED_RESOURCE_MANAGER
#include "ThreadedResourceManager.h"
#endif
using namespace gameui;
using namespace event_manager;

FPCommander *theCommander = NULL;
IosFont *storyFont; // TODO: remove

static const char * kFullScreenPref = "Config.FullScreen";

/*
 * THE MENUS
 */

/**
 * Launches a single player game
 */
void SinglePlayerGameAction::action()
{
  /*FloboStarter *starter = new StoryModeStarter(theCommander, 5, FLOBO, 0);
  starter->run(0,0,0,0,0);
  GameUIDefaults::SCREEN_STACK->push(starter);*/
}

/* Build the FPCommander */

FPCommander::FPCommander(DataPathManager *dataPathManager,
                         PreferencesManager *preferencesManager,
                         Jukebox *jukebox)
  : m_dataPathManager(dataPathManager),
    m_preferencesManager(preferencesManager),
    m_jukebox(jukebox),
    m_themeManager(new ThemeManagerImpl(*dataPathManager)),
    m_surfaceFactory(*dataPathManager),
    m_fontFactory(*dataPathManager),
    m_soundFactory(*dataPathManager),
    m_musicFactory(*dataPathManager),
    m_floboSetThemeFactory(*m_themeManager),
    m_levelThemeFactory(*m_themeManager),
    m_cursor(NULL)
{
  GTLogTrace("++");
#ifdef PRODUCE_CACHE_FILE
  cacheOutputGsl = fopen("cache.gsl", "w");
#endif
  loop = GameUIDefaults::GAME_LOOP;
  theCommander = this;

  createResourceManagers();
  GTLogTrace("audioHelper");
  m_audioHelper.reset(new AudioHelper());
  GTLogTrace("--");
}

void FPCommander::initWithGUI(bool fs)
{
  GTLogTrace("FPCommander::initWithGUI() entered");

  m_windowFramePicture = std::auto_ptr<FramePicture>(new FramePicture(25, 28, 25, 19, 26, 23));
  m_buttonIdleFramePicture = std::auto_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  m_buttonDownFramePicture = std::auto_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  m_buttonSpecialFramePicture = std::auto_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  m_buttonOverFramePicture = std::auto_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  m_textFieldIdleFramePicture = std::auto_ptr<FramePicture>(new FramePicture(5, 23, 4, 6, 10, 3));
  m_separatorFramePicture = std::auto_ptr<FramePicture>(new FramePicture(63, 2, 63, 2, 4, 2));
  m_listFramePicture = std::auto_ptr<FramePicture>(new FramePicture(5, 23, 4, 6, 10, 3));

  GTLogTrace("FPCommander::initWithGUI() init locales");

  initLocale();

  GTLogTrace("FPCommander::initWithGUI() init audio");

  initAudio();

  GTLogTrace("FPCommander::initWithGUI() init fonts");

  initFonts();

  GTLogTrace("FPCommander::initWithGUI() loading images");


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

  GTLogTrace("FPCommander::initWithGUI() configuring frames");

  m_windowFramePicture->setFrameSurface(m_frameImage);
  m_buttonIdleFramePicture->setFrameSurface(m_buttonIdleImage);
  m_buttonSpecialFramePicture->setFrameSurface(m_buttonSpecialImage);
  m_buttonDownFramePicture->setFrameSurface(m_buttonDownImage);
  m_buttonOverFramePicture->setFrameSurface(m_buttonOverImage);
  m_textFieldIdleFramePicture->setFrameSurface(m_textFieldIdleImage);
  m_separatorFramePicture->setFrameSurface(m_separatorImage);
  m_listFramePicture->setFrameSurface(m_listIdleImage);
  GTLogTrace("FPCommander::initWithGUI() completed");

}

void FPCommander::initWithoutGUI()
{
  initLocale();
}

FPCommander::~FPCommander()
{
}

extern char *dataFolder;
#include <string>

/* Initialise the default dictionnary */
void FPCommander::initLocale()
{
  locale = new LocalizedDictionary(*m_dataPathManager, "locale", "main");
}

/* Global translator */
const char * FPCommander::getLocalizedString(const char * originalString) const
{
  return locale->getLocalizedString(originalString);
}

/* Initialize the audio if necessary */
void FPCommander::initAudio()
{
    m_slideSound = getSound(FilePath("sfx").combine("slide.wav"));
    m_whipSound = getSound(FilePath("sfx").combine("whip.wav"));
    m_whopSound = getSound(FilePath("sfx").combine("whop.wav"));
    GameUIDefaults::SLIDE_SOUND = m_slideSound;
}


/* load fonts and set them for use in the GUI */
void FPCommander::initFonts()
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

    m_darkFont = getFont(fontName, 17);
    m_menuFont = getFont(fontName, 17);
    m_smallFont = getFont(fontName, 12);
    m_smallFontInfo = getFont(fontName, 12);
    m_textFont = getFont(fontName, 17);
    m_funnyFont = getFont(funnyFontName, 24);
    storyFont = getFont(fontName, 17);

    GameUIDefaults::FONT              = m_menuFont;
    GameUIDefaults::FONT_TEXT         = m_textFont;
    GameUIDefaults::FONT_INACTIVE     = m_darkFont;
    GameUIDefaults::FONT_SMALL_INFO   = m_smallFontInfo;
    GameUIDefaults::FONT_SMALL_ACTIVE = m_smallFont;
    GameUIDefaults::FONT_FUNNY        = m_funnyFont;
}

void FPCommander::initThemes()
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

String FPCommander::getFullScreenKey(void) const
{
    return String(kFullScreenPref);
}

ScreenTransitionWidget *FPCommander::createScreenTransition(Screen &fromScreen) const
{
    return new DoomMeltScreenTransitionWidget(fromScreen);
}

// Resource management
void FPCommander::cacheSurface(ImageType type, const char *path, ImageSpecialAbility specialAbility)
{
    m_surfaceResManager->cacheResource(IosSurfaceResourceKey(type, path, specialAbility));
}

IosSurfaceRef FPCommander::getSurface(ImageType type, const char *path, ImageSpecialAbility specialAbility)
{
    return m_surfaceResManager->getResource(IosSurfaceResourceKey(type, path, specialAbility));
}

IosSurfaceRef FPCommander::getSurface(ImageType type, const char *path, const ImageOperationList &list)
{
    ImageSpecialAbility specialAbility = GameUIDefaults::GAME_LOOP->getDrawContext()->guessRequiredImageAbility(list);
    return m_surfaceResManager->getResource(IosSurfaceResourceKey(type, path, specialAbility));
}

void FPCommander::cacheFont(const char *path, int size)
{
    m_fontResManager->cacheResource(IosFontResourceKey(path, size));
}

IosFontRef FPCommander::getFont(const char *path, int size)
{
    return m_fontResManager->getResource(IosFontResourceKey(path, size));
}

void FPCommander::cacheSound(const char *path)
{
    m_soundResManager->cacheResource(path);
}

SoundRef FPCommander::getSound(const char *path)
{
    return m_soundResManager->getResource(path);
}

void FPCommander::cacheMusic(const char *path)
{
    m_musicResManager->cacheResource(path);
}

MusicRef FPCommander::getMusic(const char *path)
{
    return m_musicResManager->getResource(path);
}

FloboSetThemeRef FPCommander::getFloboSetTheme(const char *name)
{
    return m_floboSetThemeResManager->getResource(name);
}

FloboSetThemeRef FPCommander::getPreferedFloboSetTheme()
{
    return getFloboSetTheme(getPreferedFloboSetThemeName().c_str());
}

const std::string &FPCommander::getPreferedFloboSetThemeName() const
{
    if (m_defaultFloboSetThemeName == "") {
        m_defaultFloboSetThemeName = m_preferencesManager->getStrPreference ("floboset_theme", getDefaultFloboSetThemeName().c_str());
    }
    return m_defaultFloboSetThemeName;
}

const std::string FPCommander::getDefaultFloboSetThemeName() const
{
    // TODO: provide a way to set the default theme in the data
    //return m_themeManager->getFloboSetThemeList()[0];
    return m_themeManager->getFloboSetThemeList()[m_themeManager->getFloboSetThemeList().size() - 1];
}


void FPCommander::setPreferedFloboSetThemeName(const char *name)
{
    m_defaultFloboSetThemeName = name;
    m_preferencesManager->setStrPreference ("floboset_theme", name);
}

const std::vector<std::string> &FPCommander::getFloboSetThemeList() const
{
    return m_themeManager->getFloboSetThemeList();
}

LevelThemeRef FPCommander::getLevelTheme(const char *name)
{
    return m_levelThemeResManager->getResource(name);
}

LevelThemeRef FPCommander::getPreferedLevelTheme(int nbPlayers)
{
    return getLevelTheme(getPreferedLevelThemeName(nbPlayers).c_str());
}

const std::string FPCommander::getPreferedLevelThemeName(int nbPlayers) const
{
    ostringstream osstream;
    osstream << "level_theme_" << nbPlayers;
    std::string result = m_preferencesManager->getStrPreference(osstream.str().c_str(), getDefaultLevelThemeName(nbPlayers).c_str());
    return result;
}

const std::string FPCommander::getDefaultLevelThemeName(int nbPlayers) const
{
    std::vector<std::string> result = m_themeManager->getLevelThemeList(nbPlayers);
    if (result.size() == 0)
        return "";
    return result[0];
}

void FPCommander::setPreferedLevelThemeName(const char *name, int nbPlayers)
{
    ostringstream osstream;
    osstream << "level_theme_" << nbPlayers;
    m_preferencesManager->setStrPreference (osstream.str().c_str(), name);
}

std::vector<std::string> FPCommander::getLevelThemeList(int nbPlayers) const
{
    return m_themeManager->getLevelThemeList(nbPlayers);
}

void FPCommander::freeUnusedResources()
{
    m_surfaceResManager->freeUnusedResources();
    //m_fontResManager->freeUnusedResources();
    m_soundResManager->freeUnusedResources();
    //m_musicResManager->freeUnusedResources();
}

void FPCommander::registerCursor(AbstractCursor *cursor)
{
    m_cursor = cursor;
}

void FPCommander::setCursorVisible(bool visible)
{
    if (m_cursor != NULL)
        m_cursor->setVisible(visible);
}

void FPCommander::createResourceManagers()
{
    GTLogTrace("++");
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
    m_floboSetThemeResManager.reset(new SimpleResourceManager<FloboSetTheme>(m_floboSetThemeFactory));
    m_levelThemeResManager.reset(new SimpleResourceManager<LevelTheme>(m_levelThemeFactory));
#endif
    GTLogTrace("--");
}

void FPCommander::playMusicTrack(const char *trackName)
{
    m_jukebox->playTrack(trackName);
}

void FPCommander::playMusicTrack()
{
    m_jukebox->playTrack();
}

void FPCommander::playSound(const char *sName, float volume, float balance)
{
    m_audioHelper->playSound(sName, volume, balance);
}

