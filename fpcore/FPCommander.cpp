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
  : _dataPathManager(dataPathManager),
    _preferencesManager(preferencesManager),
    _themeManager(new ThemeManagerImpl(*dataPathManager)),
    _surfaceFactory(*dataPathManager),
    _fontFactory(*dataPathManager),
    _soundFactory(*dataPathManager),
    _musicFactory(*dataPathManager),
    _floboSetThemeFactory(*_themeManager),
    _levelThemeFactory(*_themeManager),
    _achMgr(NULL),
    _jukebox(jukebox),
    _cursor(NULL)
{
  GTLogTrace("++");
#ifdef PRODUCE_CACHE_FILE
  cacheOutputGsl = fopen("cache.gsl", "w");
#endif
  _loop = GameUIDefaults::GAME_LOOP;
  theCommander = this;

  createResourceManagers();
  GTLogTrace("audioHelper");
  _audioHelper.reset(new AudioHelper());
  GTLogTrace("--");
}

void FPCommander::initWithGUI(bool fs)
{
  GTLogTrace("FPCommander::initWithGUI() entered");

  _windowFramePicture = std::unique_ptr<FramePicture>(new FramePicture(25, 28, 25, 19, 26, 23));
  _buttonIdleFramePicture = std::unique_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  _buttonDownFramePicture = std::unique_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  _buttonOverFramePicture = std::unique_ptr<FramePicture>(new FramePicture(13, 10, 13, 12, 7, 13));
  _textFieldIdleFramePicture = std::unique_ptr<FramePicture>(new FramePicture(5, 23, 4, 6, 10, 3));
  _separatorFramePicture = std::unique_ptr<FramePicture>(new FramePicture(63, 2, 63, 2, 4, 2));
  _listFramePicture = std::unique_ptr<FramePicture>(new FramePicture(5, 23, 4, 6, 10, 3));

  GTLogTrace("FPCommander::initWithGUI() init locales");

  initLocale();

  GTLogTrace("FPCommander::initWithGUI() init audio");

  initAudio();

  GTLogTrace("FPCommander::initWithGUI() init fonts");

  initFonts();

  GTLogTrace("FPCommander::initWithGUI() loading images");


  // Loading the frame images, and setting up the frames
  _switchOnImage = getSurface(IMAGE_RGBA, "gfx/switch-on.png");
  _switchOffImage = getSurface(IMAGE_RGBA, "gfx/switch-off.png");
  _radioOnImage = getSurface(IMAGE_RGBA, "gfx/radio-on.png");
  _radioOffImage = getSurface(IMAGE_RGBA, "gfx/radio-off.png");
  _upArrow = getSurface(IMAGE_RGBA, "gfx/uparrow.png");
  _downArrow = getSurface(IMAGE_RGBA, "gfx/downarrow.png");
  _leftArrow = getSurface(IMAGE_RGBA, "gfx/leftarrow.png");
  _rightArrow = getSurface(IMAGE_RGBA, "gfx/rightarrow.png");

  _frameImage = getSurface(IMAGE_RGBA, "gfx/frame.png");
  _buttonIdleImage = getSurface(IMAGE_RGBA, "gfx/button.png");
  _buttonDownImage = getSurface(IMAGE_RGBA, "gfx/buttondown.png");
  _buttonOverImage = getSurface(IMAGE_RGBA, "gfx/buttonover.png");
  _textFieldIdleImage = getSurface(IMAGE_RGBA, "gfx/editfield.png");
  _separatorImage = getSurface(IMAGE_RGBA, "gfx/separator.png");
  _listIdleImage = getSurface(IMAGE_RGBA, "gfx/listborder.png");

  GTLogTrace("FPCommander::initWithGUI() configuring frames");

  _windowFramePicture->setFrameSurface(_frameImage);
  _buttonIdleFramePicture->setFrameSurface(_buttonIdleImage);
  _buttonDownFramePicture->setFrameSurface(_buttonDownImage);
  _buttonOverFramePicture->setFrameSurface(_buttonOverImage);
  _textFieldIdleFramePicture->setFrameSurface(_textFieldIdleImage);
  _separatorFramePicture->setFrameSurface(_separatorImage);
  _listFramePicture->setFrameSurface(_listIdleImage);
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
  _locale = new LocalizedDictionary(*_dataPathManager, "locale", "main");
}

/* Global translator */
std::string FPCommander::getLocalizedString(const std::string & originalString) const
{
  return _locale->getLocalizedString(originalString.c_str());
}

/* Initialize the audio if necessary */
void FPCommander::initAudio()
{
    _slideSound = getSound(FilePath("sfx").combine("slide.wav"));
    _whipSound = getSound(FilePath("sfx").combine("whip.wav"));
    _whopSound = getSound(FilePath("sfx").combine("whop.wav"));
    GameUIDefaults::SLIDE_SOUND = _slideSound;
}


/* load fonts and set them for use in the GUI */
void FPCommander::initFonts()
{
    Locales_Init(); // Make sure locales are detected.
    std::string fontName, funnyFontName;
    fontName = _locale->getLocalizedString("__FONT__");
    _localizedFontName = fontName;
    funnyFontName = "gfx/zill_spills.ttf";
    /*try {
     font = dataPathManager.getPath(locale->getLocalizedString("__FONT__"));
     }
     catch (const std::exception &) {
     fprintf(stderr,"Font %s not found.\n", (const char *)font);
     font = dataPathManager.getPath("gfx/font.ttf");
     fprintf(stderr,"Using default font %s.\n", (const char *)font);
     }
     funny_path = getDataPathManager().getPath("gfx/zill_spills.ttf");*/

    _darkFont = getFont(fontName, 17);
    _menuFont = getFont(fontName, 17);
    _smallFont = getFont(fontName, 12);
    _smallFontInfo = getFont(fontName, 12);
    _textFont = getFont(fontName, 17);
    _funnyFont = getFont(funnyFontName, 24);
    storyFont = getFont(fontName, 17);

    GameUIDefaults::FONT              = _menuFont;
    GameUIDefaults::FONT_TEXT         = _textFont;
    GameUIDefaults::FONT_INACTIVE     = _darkFont;
    GameUIDefaults::FONT_SMALL_INFO   = _smallFontInfo;
    GameUIDefaults::FONT_SMALL_ACTIVE = _smallFont;
    GameUIDefaults::FONT_FUNNY        = _funnyFont;
    GameUIDefaults::FONT_COLOR = RGBA(0xff, 0xf9, 0xa6, 0xff);
    GameUIDefaults::FONT_INACTIVE_COLOR = RGBA(0xad, 0xa1, 0x50, 0xff);
}

void FPCommander::initThemes()
{
#ifdef DISABLED
    // List the themes in the various pack folders
    std::vector<String> themeFolders = getDataPathManager().getEntriesAtPath("theme");
    // Load the themes from the list (only those matching the correct extension)
    for (int i = 0 ; i < static_cast<int>(themeFolders.size()) ; i++) {
        if (ios_fc::substring(themeFolders[i], themeFolders[i].size() - 8)
            == themeFolderExtension) {
            cout << "Theme to be loaded: " << (const char *)(themeFolders[i]); << endl;
            //loadTheme(themeFolders[i]);
        }
    }
#endif
}

std::string FPCommander::getFullScreenKey(void) const
{
    return kFullScreenPref;
}

ScreenTransitionWidget *FPCommander::createScreenTransition(Screen &fromScreen) const
{
    return new DoomMeltScreenTransitionWidget(fromScreen);
}

// Resource management
void FPCommander::cacheSurface(ImageType type, const std::string &path, ImageSpecialAbility specialAbility)
{
    _surfaceResManager->cacheResource(IosSurfaceResourceKey(type, path, specialAbility));
}

IosSurfaceRef FPCommander::getSurface(ImageType type, const std::string &path, ImageSpecialAbility specialAbility)
{
    return _surfaceResManager->getResource(IosSurfaceResourceKey(type, path, specialAbility));
}

IosSurfaceRef FPCommander::getSurface(ImageType type, const std::string &path, const ImageOperationList &list)
{
    ImageSpecialAbility specialAbility = GameUIDefaults::GAME_LOOP->getDrawContext()->guessRequiredImageAbility(list);
    return _surfaceResManager->getResource(IosSurfaceResourceKey(type, path, specialAbility));
}

void FPCommander::cacheFont(const std::string &path, int size)
{
    _fontResManager->cacheResource(IosFontResourceKey(path, size));
}

IosFontRef FPCommander::getFont(const std::string &path, int size)
{
    return _fontResManager->getResource(IosFontResourceKey(path, size));
}

void FPCommander::cacheSound(const std::string &path)
{
    _soundResManager->cacheResource(path);
}

SoundRef FPCommander::getSound(const std::string &path)
{
    return _soundResManager->getResource(path);
}

void FPCommander::cacheMusic(const std::string &path)
{
    _musicResManager->cacheResource(path);
}

MusicRef FPCommander::getMusic(const std::string &path)
{
    return _musicResManager->getResource(path);
}

FloboSetThemeRef FPCommander::getFloboSetTheme(const std::string &name)
{
    return _floboSetThemeResManager->getResource(name);
}

FloboSetThemeRef FPCommander::getPreferedFloboSetTheme()
{
    return getFloboSetTheme(getPreferedFloboSetThemeName().c_str());
}

const std::string &FPCommander::getPreferedFloboSetThemeName() const
{
    if (_defaultFloboSetThemeName == "") {
        _defaultFloboSetThemeName = _preferencesManager->getStrPreference ("floboset_theme", getDefaultFloboSetThemeName().c_str());
    }
    return _defaultFloboSetThemeName;
}

const std::string FPCommander::getDefaultFloboSetThemeName() const
{
    // TODO: provide a way to set the default theme in the data
    //return _themeManager->getFloboSetThemeList()[0];
    return _themeManager->getFloboSetThemeList()[_themeManager->getFloboSetThemeList().size() - 1];
}


void FPCommander::setPreferedFloboSetThemeName(const std::string &name)
{
    _defaultFloboSetThemeName = name;
    _preferencesManager->setStrPreference ("floboset_theme", name.c_str());
}

const std::vector<std::string> &FPCommander::getFloboSetThemeList() const
{
    return _themeManager->getFloboSetThemeList();
}

LevelThemeRef FPCommander::getLevelTheme(const std::string &name)
{
    return _levelThemeResManager->getResource(name);
}

LevelThemeRef FPCommander::getPreferedLevelTheme(int nbPlayers)
{
    return getLevelTheme(getPreferedLevelThemeName(nbPlayers).c_str());
}

const std::string FPCommander::getPreferedLevelThemeName(int nbPlayers) const
{
    ostringstream osstream;
    osstream << "level_theme_" << nbPlayers;
    std::string result = _preferencesManager->getStrPreference(osstream.str().c_str(), getDefaultLevelThemeName(nbPlayers).c_str());
    return result;
}

const std::string FPCommander::getDefaultLevelThemeName(int nbPlayers) const
{
    std::vector<std::string> result = _themeManager->getLevelThemeList(nbPlayers);
    if (result.size() == 0)
        return "";
    return result[0];
}

void FPCommander::setPreferedLevelThemeName(const std::string &name, int nbPlayers)
{
    ostringstream osstream;
    osstream << "level_theme_" << nbPlayers;
    _preferencesManager->setStrPreference (osstream.str().c_str(), name.c_str());
}

std::vector<std::string> FPCommander::getLevelThemeList(int nbPlayers) const
{
    return _themeManager->getLevelThemeList(nbPlayers);
}

void FPCommander::freeUnusedResources()
{
    _surfaceResManager->freeUnusedResources();
    //_fontResManager->freeUnusedResources();
    _soundResManager->freeUnusedResources();
    //_musicResManager->freeUnusedResources();
}

void FPCommander::registerCursor(AbstractCursor *cursor)
{
    _cursor = cursor;
}

void FPCommander::setCursorVisible(bool visible)
{
    if (_cursor != NULL)
        _cursor->setVisible(visible);
}

void FPCommander::createResourceManagers()
{
    GTLogTrace("++");
#ifdef THREADED_RESOURCE_MANAGER
    _surfaceResManager.reset(new ThreadedResourceManager<IosSurface, IosSurfaceResourceKey>(_surfaceFactory));
    _fontResManager.reset(new ThreadedResourceManager<IosFont, IosFontResourceKey>(_fontFactory));
    _soundResManager.reset(new ThreadedResourceManager<audio_manager::Sound>(_soundFactory));
    _musicResManager.reset(new ThreadedResourceManager<audio_manager::Music>(_musicFactory));
#else
    _surfaceResManager.reset(new SimpleResourceManager<IosSurface, IosSurfaceResourceKey>(_surfaceFactory));
    _fontResManager.reset(new SimpleResourceManager<IosFont, IosFontResourceKey>(_fontFactory));
    _soundResManager.reset(new SimpleResourceManager<audio_manager::Sound>(_soundFactory));
    _musicResManager.reset(new SimpleResourceManager<audio_manager::Music>(_musicFactory));
    _floboSetThemeResManager.reset(new SimpleResourceManager<FloboSetTheme>(_floboSetThemeFactory));
    _levelThemeResManager.reset(new SimpleResourceManager<LevelTheme>(_levelThemeFactory));
#endif
    GTLogTrace("--");
}

void FPCommander::playMusicTrack(const std::string &trackName)
{
    _jukebox->playTrack(trackName);
}

void FPCommander::playMusicTrack()
{
    _jukebox->playTrack();
}

void FPCommander::playSound(const std::string &sName, float volume, float balance)
{
    _audioHelper->playSound(sName, volume, balance);
}
