/* FloboPuyo
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gyom@ios-software.com>
 *
 * iOS Software <http://www.ios-software.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 */
#ifndef _PUYOSINGLEPLAYERSTARTER
#define _PUYOSINGLEPLAYERSTARTER

#include "GameStateMachine.h"
#include "LocalGameStates.h"
#include "PuyoStarter.h"
#include "HiScores.h"
#include "StatsWidget.h"
#include "goomsl/goomsl.h"
#include "goomsl/goomsl_hash.h"
#include <vector>

class SinglePlayerGameWidget : public GameWidget, public Action {
public:
    SinglePlayerGameWidget(int lifes, String aiFace);
    void initWithGUI(PuyoView &areaA, PuyoView &areaB, PuyoPlayer &playercontroller, LevelTheme &levelTheme, int level, Action *gameOverAction);
    virtual ~SinglePlayerGameWidget();
    bool didPlayerWon() const { return isGameARunning(); }
    void cycle();
    StoryWidget *getOpponent();
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event);
protected:
    PuyoIA *opponentcontroller;
    int faceTicks;
    StoryWidget opponent;
    CheatCodeManager killLeftCheat, killRightCheat;
};

class SinglePlayerStandardLayoutGameWidget : public SinglePlayerGameWidget
{
public:
    SinglePlayerStandardLayoutGameWidget(PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme, int level, int nColors, int lifes, String aiFace, Action *gameOverAction = NULL);
private:
    PuyoSetTheme &attachedPuyoThemeSet;
    PuyoRandomSystem attachedRandom;
    PuyoLocalGameFactory attachedGameFactory;
    PuyoView areaA, areaB;
    PuyoCombinedEventPlayer playercontroller;
};

/**
 * This class provides objects that are used during the game,
 * such as the player name, the player controller,
 * and the screen layout.
 */
class SinglePlayerFactory {
public:
    virtual String getPlayerName() const = 0;
    virtual SinglePlayerGameWidget *createGameWidget
        (PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme,
         int level, int nColors, int lifes, String aiFace,
         Action *gameOverAction) = 0;
    virtual StatsWidgetDimensions getStatsWidgetDimensions() const = 0;
    virtual ~SinglePlayerFactory() {};
};

/**
 * Implementation of SinglePlayerFactory with the standard game layout
 */
class SinglePlayerStandardLayoutFactory : public SinglePlayerFactory {
    virtual SinglePlayerGameWidget *createGameWidget
    (PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme,
     int level, int nColors, int lifes, String aiFace,
     Action *gameOverAction);
    virtual StatsWidgetDimensions getStatsWidgetDimensions() const;
};

class PuyoLevelDefinitions {
public:
    struct SelIA {
      SelIA(int level, int nColors);
        int level;
        int nColors;
    };
    struct LevelDefinition {
      LevelDefinition(String levelName, String introStory,
		      String opponentStory, String opponentName, String opponent,
              String backgroundTheme, String gameLostStory, String gameOverStory,
		      SelIA easySettings, SelIA mediumSettings, SelIA hardSettings)
	: levelName(levelName), introStory(introStory),
	   opponentStory(opponentStory),  opponentName(opponentName), opponent(opponent),
       backgroundTheme(backgroundTheme), gameLostStory(gameLostStory), gameOverStory(gameOverStory),
	   easySettings(easySettings), mediumSettings(mediumSettings), hardSettings(hardSettings) {}
      String levelName;
      String introStory;
      String opponentStory;
      String opponentName;
      String opponent;
      String backgroundTheme;
      String gameLostStory;
      String gameOverStory;
      SelIA easySettings;
      SelIA mediumSettings;
      SelIA hardSettings;
    };
    PuyoLevelDefinitions(String levelDefinitionFile);
    LevelDefinition *getLevelDefinition(int levelNumber) { return levelDefinitions[levelNumber]; }
    int getNumLevels() const { return levelDefinitions.size(); }
    virtual ~PuyoLevelDefinitions();
private:
    void addLevelDefinition(String levelName, String introStory,
			    String opponentStory, String opponentName, String opponent,
                String backgroundTheme, String gameLostStory, String gameOverStory,
			    SelIA easySettings,
			    SelIA mediumSettings, SelIA hardSettings);
    static void end_level(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void get_BoolPreference(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static PuyoLevelDefinitions *currentDefinition;
    AdvancedBuffer<LevelDefinition *> levelDefinitions;
};

class PuyoSingleGameLevelData {
public:
    PuyoSingleGameLevelData(int gameLevel, int difficulty, PuyoLevelDefinitions &levelDefinitions);
    ~PuyoSingleGameLevelData();
    String getIntroStory() const;
    String getStory() const;
    String getGameLostStory() const;
    String getGameOverStory() const;
    PuyoSetTheme &getPuyoTheme() const;
    LevelTheme &getLevelTheme() const;
    int getIALevel() const;
    String getIAName() const;
    String getIAFace() const;
    int getNColors() const;
    GameOptions getGameOptions() const;
private:
    int gameLevel, difficulty;
    PuyoLevelDefinitions::LevelDefinition *levelDefinition;
    PuyoSetTheme *themeToUse;
    LevelTheme *levelThemeToUse;
};

class PuyoGameOver1PScreen : public StoryScreen {
public:
    PuyoGameOver1PScreen(String screenName, Screen &previousScreen, Action *finishedAction,
            String playerName, const PlayerGameStat &playerPoints, bool initialTransition=false);
    void refresh();
    virtual ~PuyoGameOver1PScreen();
private:
	HBox titleBox;
	Text titleText;
	Text titleScore;
    Text names[kHiScoresNumber], points[kHiScoresNumber];
    VBox hiScoreNameBox, hiScorePointBox;
    HBox hiScoreBox;
    String playerName;
    PlayerGameStat playerStat;
};

class SinglePlayerMatch;

/**
 * State machine managing a single player game session
 */
class SinglePlayerStarterAction : public Action {
public:
    SinglePlayerStarterAction(MainScreen *mainScreen, int difficulty,
			      SinglePlayerFactory *spFactory);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			event_manager::GameControlEvent *event);
protected:
    virtual void stateMachine();
    virtual void performMatchPlaying(bool skipIntroduction = false,
			     bool popScreen = false);
    virtual void performEndOfMatch();
    virtual void performHiScoreScreen(String gameOverStoryName);
    virtual void performBackToMenu();
    virtual void performGameWon();

    enum State {
      kGameNotStarted,
      kMatchPlaying,
      kGameWon,
      kGameOver,
      kHiScoreScreen
    };
    MainScreen *m_mainScreen;
    State m_state;
    SinglePlayerFactory *m_spFactory;
    int m_currentLevel, m_lifes, m_difficulty;
    SinglePlayerMatch *m_currentMatch;
    PuyoGameOver1PScreen *m_hiScoreScreen;
    StoryScreen *m_gameWonScreen;
    PuyoLevelDefinitions m_levelDefinitions;
    PlayerGameStat m_playerStat;
};

/**
 * State machine managing a single player match against an opponent
 */
class SinglePlayerMatch : public Action, public Widget {
public:
    SinglePlayerMatch(PlayerGameStat &playerStat,
              Action *gameOverAction,
		      PuyoSingleGameLevelData *levelData,
		      bool skipIntroduction = false,
                      bool popScreen = false,
		      SinglePlayerFactory *spFactory = NULL,
		      int remainingLifes = 0);
    virtual ~SinglePlayerMatch();
    void run();
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			event_manager::GameControlEvent *event);
    enum State {
      kNotRunning,
      kStoryIntroduction,
      kStory,
      kMatchGettingStarted,
      kMatchPlaying,
      kMatchLostAnimation,
      kMatchWonScores,
      kMatchLostScores,
      kMatchOverWon,
      kMatchOverLost,
      kMatchOverAborted
    };
    /**
     * Returns the current state of the match
     */
    State getState() const { return m_state; }
    /**
     * Returns the game over story name of the opponent
     */
    String getGameOverStoryName() const
      { return m_levelData->getGameOverStory(); }
private:
    void performStoryIntroduction();
    void performOpponentStory();
    void performMatchPrepare();
    void performMatchStart();
    void performEndOfMatch();
    void performMatchLostAnimation();
    void performMatchScores(State scoreState);
    void trigMatchOverAction();
    void prepareGame();
    /**
     * Performs a step in the match state machine
     */
    void stateMachine();
    State m_state;
    Action *m_matchOverAction;
    PuyoSingleGameLevelData *m_levelData;
    bool m_skipIntroduction;
    bool m_popScreen;
    SinglePlayerFactory *m_spFactory;
    int m_remainingLifes;
    StoryScreen *m_introStory, *m_opponentStory;
    GameScreen *m_gameScreen;
    SinglePlayerGameWidget *m_gameWidget;
    StoryWidget *m_getReadyWidget, *m_matchLostAnimation;
    TwoPlayersStatsWidget *m_statsWidget;
    PlayerGameStat &m_playerStat;
};

class AltTweakedGameWidgetFactory : public GameWidgetFactory,
                                    public PlayerNameProvider
{
public:
    AltTweakedGameWidgetFactory(PuyoLevelDefinitions::LevelDefinition *levelDef);
public:
    virtual String getPlayerName(int playerNumber) const
    { return "bob"; }
public:
    virtual GameWidget *createGameWidget(PuyoSetTheme &puyoThemeSet,
                                         LevelTheme &levelTheme,
                                         String centerFace,
                                         Action *gameOverAction);
private:
    PuyoLevelDefinitions::LevelDefinition *m_levelDef;
};

struct SharedGameAssets
{
    std::string playerName;
    int difficulty;
    PuyoLevelDefinitions::LevelDefinition *levelDef;
};

/**
 * Display the animation when the match is over (1P mode)
 */
class SinglePlayerMatchIsOverState : public GameState, public Action
{
public:
    SinglePlayerMatchIsOverState(SharedGameAssets *sharedGameAssets,
				 SharedMatchAssets *sharedMatchAssets);
    // GameState implementation
    virtual void enterState();
    virtual void exitState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // Action implementation
    virtual void action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event);
    // Own methods
    void setNextState(GameState *nextState) {
        m_nextState = nextState;
    }
private:
    SharedGameAssets *m_sharedGameAssets;
    SharedMatchAssets *m_sharedMatchAssets;
    std::auto_ptr<StoryWidget> m_gameLostWidget;
    bool m_aknowledged;
    GameState *m_nextState;
};

/**
 * Sub-state machine implementing the behaviour of a single match
 * against a computer opponent in single-player mode
 */
class SinglePlayerMatchState : public GameState,
                               public GameWidgetFactory,
                               public PlayerNameProvider,
                               public Action
{
public:
    SinglePlayerMatchState(SharedGameAssets *sharedGameAssets);
    // GameState implementation
    virtual void enterState();
    virtual void exitState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // PlayerNameProvider implementation
    virtual String getPlayerName(int playerNumber) const;
    // GameWidgetFactory implementation
    virtual GameWidget *createGameWidget(PuyoSetTheme &puyoThemeSet,
                                         LevelTheme &levelTheme,
                                         String centerFace,
                                         Action *gameOverAction);
    // Action implementation
    virtual void action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event);
    // Own methods
    void setAbortedState(GameState *abortedState) {
        m_abortedState = abortedState;
    }
    void setVictoriousState(GameState *victoriousState) {
        m_victoriousState = victoriousState;
    }
    void setHumiliatedState(GameState *humiliatedState) {
        m_humiliatedState = humiliatedState;
    }
private:
    enum {
        LEAVE_MATCH,
        ABORT_GAME
    };
    GameStateMachine m_stateMachine;
    SharedGameAssets *m_sharedGameAssets;
    SharedMatchAssets m_sharedAssets;
    SharedGetReadyAssets m_sharedGetReadyAssets;
    GameState *m_nextState;
    GameState *m_abortedState, *m_victoriousState, *m_humiliatedState;

    std::auto_ptr<DisplayStoryScreenState> m_introStoryScreen;
    std::auto_ptr<DisplayStoryScreenState> m_opponentStoryScreen;
    std::auto_ptr<SetupMatchState>       m_setupMatch;
    std::auto_ptr<EnterPlayerReadyState> m_enterPlayersReady;
    std::auto_ptr<ExitPlayerReadyState>  m_exitPlayersReady;
    std::auto_ptr<MatchPlayingState>     m_matchPlaying;
    std::auto_ptr<SinglePlayerMatchIsOverState> m_matchIsOver;
    std::auto_ptr<DisplayStatsState>     m_displayStats;
    std::auto_ptr<CallActionState>       m_abortGame;
    std::auto_ptr<CallActionState>       m_leaveMatch;
};

//class SinglePlayerMatchStateMachine : public GameStateMachine
class AltSinglePlayerStarterAction : public Action {
public:
    AltSinglePlayerStarterAction(MainScreen *mainScreen, int difficulty,
                                  SinglePlayerFactory *spFactory);//(int difficulty, PlayerNameProvider *nameProvider = NULL);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			event_manager::GameControlEvent *event);
private:
    GameStateMachine m_stateMachine;
    std::auto_ptr<PuyoLevelDefinitions> m_levelDefinitions;
    std::auto_ptr<AltTweakedGameWidgetFactory> m_gameWidgetFactory;
    SharedGameAssets     m_sharedGameAssets;

    std::auto_ptr<SinglePlayerMatchState> m_playMatch;
    std::auto_ptr<LeaveGameState>         m_leaveGame;
};

#endif // _PUYOSINGLEPLAYERSTARTER


