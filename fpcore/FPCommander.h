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
#include <memory>

using namespace gameui;


class FPApplicationState
{
public:
    enum UIState {
		IN_GAME, IN_MENU
	};
	FPApplicationState() : ui(IN_MENU), difficulty(0), level(0), playerScore(0) {}

	UIState ui;
	int difficulty;
	int level;
	int playerScore;
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

    String getFullScreenKey(void) const;

    // Cursor management
    void registerCursor(AbstractCursor *cursor);
    void setCursorVisible(bool visible);

    // Transition widget factory
    virtual ScreenTransitionWidget *createScreenTransition(Screen &fromScreen) const;

    // Resource managers
    void cacheSurface(ImageType type, const char *path, ImageSpecialAbility specialAbility = 0);
    IosSurfaceRef getSurface(ImageType type, const char *path, ImageSpecialAbility specialAbility = 0);
    IosSurfaceRef getSurface(ImageType type, const char *path, const ImageOperationList &list);
    void cacheFont(const char *path, int size);
    IosFontRef getFont(const char *path, int size);
    void cacheSound(const char *path);
    SoundRef getSound(const char *path);
    void cacheMusic(const char *path);
    MusicRef getMusic(const char *path);

    FloboSetThemeRef getFloboSetTheme(const char *name);
    FloboSetThemeRef getPreferedFloboSetTheme();
    const std::string &getPreferedFloboSetThemeName() const;
    void setPreferedFloboSetThemeName(const char *name);
    const std::vector<std::string> &getFloboSetThemeList() const;
    virtual const std::string getDefaultFloboSetThemeName() const;

    LevelThemeRef getLevelTheme(const char *name);
    LevelThemeRef getPreferedLevelTheme(int nbPlayers = 2);
    const std::string getPreferedLevelThemeName(int nbPlayers = 2) const;
    void setPreferedLevelThemeName(const char *name, int nbPlayers = 2);
    std::vector<std::string> getLevelThemeList(int nbPlayers = 2) const;
    virtual const std::string getDefaultLevelThemeName(int nbPlayers = 2) const;

    void freeUnusedResources();

    // Data path management
    const DataPathManager &getDataPathManager() { return *m_dataPathManager; }
    const char * getLocalizedString(const char * originalString) const;
    // Preferences management
    PreferencesManager *getPreferencesManager() const { return m_preferencesManager; }
    // Achievements management
    virtual AchievementsManager *getAchievementsManager() const { return m_achMgr; }
    void setAchievementsManager(AchievementsManager *achMgr) { m_achMgr = achMgr; }
    // Common resources accessor
    const FramePicture *getWindowFramePicture() const { return m_windowFramePicture.get(); }
    const FramePicture *getButtonFramePicture() const { return m_buttonIdleFramePicture.get(); }
    const FramePicture *getButtonDownFramePicture() const { return m_buttonDownFramePicture.get(); }
    const FramePicture *getButtonOverFramePicture() const { return m_buttonOverFramePicture.get(); }
    const FramePicture *getEditFieldFramePicture() const { return m_textFieldIdleFramePicture.get(); }
    const FramePicture *getEditFieldOverFramePicture() const { return m_textFieldIdleFramePicture.get(); }
    const FramePicture *getSeparatorFramePicture() const { return m_separatorFramePicture.get(); }
    const FramePicture *getListFramePicture() const { return m_listFramePicture.get(); }
    IosSurface * getSwitchOnPicture() { return m_switchOnImage; }
    IosSurface * getSwitchOffPicture() { return m_switchOffImage; }
    IosSurface * getRadioOnPicture() { return m_radioOnImage; }
    IosSurface * getRadioOffPicture() { return m_radioOffImage; }
    IosSurface * getUpArrow() { return m_upArrow; }
    IosSurface * getDownArrow() { return m_downArrow; }
    IosSurface * getLeftArrow() { return m_leftArrow; }
    IosSurface * getRightArrow() { return m_rightArrow; }
    audio_manager::Sound * getWhipSound() const { return m_whipSound; }
    audio_manager::Sound * getWhopSound() const { return m_whopSound; }
    const String &getLocalizedFontName() const { return m_localizedFontName; }

    void playMusicTrack(const char *trackName);
    void playMusicTrack();
    void playSound(const char *sName, float volume = 1.0, float balance = 0.0f);

	// Application state
	const FPApplicationState &appState() const { return m_appState; }
	FPApplicationState &appState() { return m_appState; }

  protected:
    // Resource manager factory
    virtual void createResourceManagers();
    // Font creation function
    virtual void initFonts();
    // Theme initialisation function
    virtual void initThemes();
    // Data path management
    DataPathManager *m_dataPathManager;
    PreferencesManager *m_preferencesManager;
    // Resource Managers
    std::auto_ptr<ThemeManager> m_themeManager;
    IosSurfaceFactory m_surfaceFactory;
    std::auto_ptr<IosSurfaceResourceManager> m_surfaceResManager;
    IosFontFactory m_fontFactory;
    std::auto_ptr<IosFontResourceManager> m_fontResManager;
    SoundFactory m_soundFactory;
    std::auto_ptr<SoundResourceManager> m_soundResManager;
    MusicFactory m_musicFactory;
    std::auto_ptr<MusicResourceManager> m_musicResManager;
    FloboSetThemeFactory m_floboSetThemeFactory;
    std::auto_ptr<FloboSetThemeResourceManager> m_floboSetThemeResManager;
    LevelThemeFactory m_levelThemeFactory;
    std::auto_ptr<LevelThemeResourceManager> m_levelThemeResManager;
    // Localization management
    LocalizedDictionary * locale;
    AchievementsManager *m_achMgr;
  private:

    friend class SinglePlayerGameAction;
    friend class NetGameAction;
    friend class MainMenu;
    friend class NetworkGameMenu;

    void initLocale();
    void initAudio();

    GameLoop   *loop;
    auto_ptr<AudioHelper> m_audioHelper;
    Jukebox *m_jukebox;
protected:
    IosSurfaceRef m_frameImage;
    IosSurfaceRef m_buttonIdleImage, m_buttonDownImage, m_buttonOverImage;
    IosSurfaceRef m_textFieldIdleImage;
    IosSurfaceRef m_separatorImage;
    IosSurfaceRef m_listIdleImage;
    IosSurfaceRef m_switchOnImage, m_switchOffImage;
    IosSurfaceRef m_radioOnImage, m_radioOffImage;
    IosSurfaceRef m_upArrow, m_downArrow, m_leftArrow, m_rightArrow;
    std::auto_ptr<FramePicture> m_windowFramePicture;
    std::auto_ptr<FramePicture> m_buttonIdleFramePicture;
    std::auto_ptr<FramePicture> m_buttonDownFramePicture;
    std::auto_ptr<FramePicture> m_buttonOverFramePicture;
    std::auto_ptr<FramePicture> m_textFieldIdleFramePicture;
    std::auto_ptr<FramePicture> m_separatorFramePicture;
    std::auto_ptr<FramePicture> m_listFramePicture;

    String m_localizedFontName;
    IosFontRef m_darkFont;
    IosFontRef m_menuFont;
    IosFontRef m_smallFont;
    IosFontRef m_smallFontInfo;
    IosFontRef m_textFont;
    IosFontRef m_funnyFont;

    SoundRef m_slideSound;
    SoundRef m_whipSound;
    SoundRef m_whopSound;

    AbstractCursor *m_cursor;
	FPApplicationState m_appState;

    mutable std::string m_defaultFloboSetThemeName;
};

extern class FPCommander *theCommander;

#endif // _PUYOCOMMANDER
