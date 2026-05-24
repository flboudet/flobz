#ifndef _PUYOCOMMANDER
#define _PUYOCOMMANDER

#include "gameui.h"
#include "Frame.h"
#include "DataPathManager.h"
#include "LocalizedDictionary.h"
#include "GameCursor.h"
#include "ScreenTransition.h"
#include "audio.h"
#include "Theme.h"
#include "FPResources.h"
#include "PreferencesManager.h"
#include "AchievementsManager.h"
#include <string>
#include <memory>

using namespace gameui;


class FPApplicationState
{
public:
    enum UIState {
		IN_GAME, IN_MENU
	};
	FPApplicationState() : _ui(IN_MENU), _difficulty(0), _level(0), _playerScore(0) {}

	UIState _ui;
	int _difficulty;
	int _level;
	int _playerScore;
};

class FPCommander
{
  public:
    FPCommander(DataPathManager *dataPathManager,
                PreferencesManager *preferencesManager,
                Jukebox *jukebox);
    void initWithGUI(bool fullscreen);
    void initWithoutGUI();

    virtual ~FPCommander();

    std::string getFullScreenKey(void) const;

    // Cursor management
    void registerCursor(AbstractCursor *cursor);
    void setCursorVisible(bool visible);

    // Transition widget factory
    virtual ScreenTransitionWidget *createScreenTransition(Screen &fromScreen) const;

    // Resource managers
    void cacheSurface(ImageType type, const std::string &path, ImageSpecialAbility specialAbility = 0);
    IosSurfaceRef getSurface(ImageType type, const std::string &path, ImageSpecialAbility specialAbility = 0);
    IosSurfaceRef getSurface(ImageType type, const std::string &path, const ImageOperationList &list);
    void cacheFont(const std::string &path, int size);
    IosFontRef getFont(const std::string &path, int size);
    void cacheSound(const std::string &path);
    SoundRef getSound(const std::string &path);
    void cacheMusic(const std::string &path);
    MusicRef getMusic(const std::string &path);

    FloboSetThemeRef getFloboSetTheme(const std::string &name);
    FloboSetThemeRef getPreferedFloboSetTheme();
    const std::string &getPreferedFloboSetThemeName() const;
    void setPreferedFloboSetThemeName(const std::string &name);
    const std::vector<std::string> &getFloboSetThemeList() const;
    virtual const std::string getDefaultFloboSetThemeName() const;

    LevelThemeRef getLevelTheme(const std::string &name);
    LevelThemeRef getPreferedLevelTheme(int nbPlayers = 2);
    const std::string getPreferedLevelThemeName(int nbPlayers = 2) const;
    void setPreferedLevelThemeName(const std::string &name, int nbPlayers = 2);
    std::vector<std::string> getLevelThemeList(int nbPlayers = 2) const;
    virtual const std::string getDefaultLevelThemeName(int nbPlayers = 2) const;

    void freeUnusedResources();

    // Data path management
    const DataPathManager &getDataPathManager() { return *_dataPathManager; }
    std::string getLocalizedString(const std::string & originalString) const;
    // Preferences management
    PreferencesManager *getPreferencesManager() const { return _preferencesManager; }
    // Achievements management
    virtual AchievementsManager *getAchievementsManager() const { return _achMgr; }
    void setAchievementsManager(AchievementsManager *achMgr) { _achMgr = achMgr; }
    // Common resources accessor
    const FramePicture *getWindowFramePicture() const { return _windowFramePicture.get(); }
    const FramePicture *getButtonFramePicture() const { return _buttonIdleFramePicture.get(); }
    const FramePicture *getButtonDownFramePicture() const { return _buttonDownFramePicture.get(); }
    const FramePicture *getButtonOverFramePicture() const { return _buttonOverFramePicture.get(); }
    const FramePicture *getEditFieldFramePicture() const { return _textFieldIdleFramePicture.get(); }
    const FramePicture *getEditFieldOverFramePicture() const { return _textFieldIdleFramePicture.get(); }
    const FramePicture *getSeparatorFramePicture() const { return _separatorFramePicture.get(); }
    const FramePicture *getListFramePicture() const { return _listFramePicture.get(); }
    IosSurface * getSwitchOnPicture() { return _switchOnImage; }
    IosSurface * getSwitchOffPicture() { return _switchOffImage; }
    IosSurface * getRadioOnPicture() { return _radioOnImage; }
    IosSurface * getRadioOffPicture() { return _radioOffImage; }
    IosSurface * getUpArrow() { return _upArrow; }
    IosSurface * getDownArrow() { return _downArrow; }
    IosSurface * getLeftArrow() { return _leftArrow; }
    IosSurface * getRightArrow() { return _rightArrow; }
    audio_manager::Sound * getWhipSound() const { return _whipSound; }
    audio_manager::Sound * getWhopSound() const { return _whopSound; }
    const std::string &getLocalizedFontName() const { return _localizedFontName; }

    void playMusicTrack(const std::string &trackName);
    void playMusicTrack();
    void playSound(const std::string &sName, float volume = 1.0, float balance = 0.0f);

	// Application state
	const FPApplicationState &appState() const { return _appState; }
	FPApplicationState &appState() { return _appState; }

  protected:
    // Resource manager factory
    virtual void createResourceManagers();
    // Font creation function
    virtual void initFonts();
    // Theme initialisation function
    virtual void initThemes();
    // Data path management
    DataPathManager *_dataPathManager;
    PreferencesManager *_preferencesManager;
    // Resource Managers
    std::unique_ptr<ThemeManager> _themeManager;
    IosSurfaceFactory _surfaceFactory;
    std::unique_ptr<IosSurfaceResourceManager> _surfaceResManager;
    IosFontFactory _fontFactory;
    std::unique_ptr<IosFontResourceManager> _fontResManager;
    SoundFactory _soundFactory;
    std::unique_ptr<SoundResourceManager> _soundResManager;
    MusicFactory _musicFactory;
    std::unique_ptr<MusicResourceManager> _musicResManager;
    FloboSetThemeFactory _floboSetThemeFactory;
    std::unique_ptr<FloboSetThemeResourceManager> _floboSetThemeResManager;
    LevelThemeFactory _levelThemeFactory;
    std::unique_ptr<LevelThemeResourceManager> _levelThemeResManager;
    // Localization management
    LocalizedDictionary * _locale;
    AchievementsManager *_achMgr;
  private:

    friend class SinglePlayerGameAction;
    friend class NetGameAction;
    friend class MainMenu;
    friend class NetworkGameMenu;

    void initLocale();
    void initAudio();

    GameLoop   *_loop;
    unique_ptr<AudioHelper> _audioHelper;
    Jukebox *_jukebox;
protected:
    IosSurfaceRef _frameImage;
    IosSurfaceRef _buttonIdleImage, _buttonDownImage, _buttonOverImage;
    IosSurfaceRef _textFieldIdleImage;
    IosSurfaceRef _separatorImage;
    IosSurfaceRef _listIdleImage;
    IosSurfaceRef _switchOnImage, _switchOffImage;
    IosSurfaceRef _radioOnImage, _radioOffImage;
    IosSurfaceRef _upArrow, _downArrow, _leftArrow, _rightArrow;
    std::unique_ptr<FramePicture> _windowFramePicture;
    std::unique_ptr<FramePicture> _buttonIdleFramePicture;
    std::unique_ptr<FramePicture> _buttonDownFramePicture;
    std::unique_ptr<FramePicture> _buttonOverFramePicture;
    std::unique_ptr<FramePicture> _textFieldIdleFramePicture;
    std::unique_ptr<FramePicture> _separatorFramePicture;
    std::unique_ptr<FramePicture> _listFramePicture;

    std::string _localizedFontName;
    IosFontRef _darkFont;
    IosFontRef _menuFont;
    IosFontRef _smallFont;
    IosFontRef _smallFontInfo;
    IosFontRef _textFont;
    IosFontRef _funnyFont;

    SoundRef _slideSound;
    SoundRef _whipSound;
    SoundRef _whopSound;

    AbstractCursor *_cursor;
	FPApplicationState _appState;

    mutable std::string _defaultFloboSetThemeName;
};

extern class FPCommander *theCommander;

#endif // _PUYOCOMMANDER
