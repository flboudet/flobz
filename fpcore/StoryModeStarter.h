/* FloboPop
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
#ifndef _STORYMODESTARTER_H_
#define _STORYMODESTARTER_H_

#include "GameStateMachine.h"
#include "LocalGameStates.h"
#include "AIPlayer.h"
#include "EventPlayer.h"
#include "HiScores.h"
#include "StatsWidget.h"
#include "goomsl.h"
#include "goomsl_hash.h"
#include <vector>

class StoryModeGameWidget : public GameWidget2P, public Action {
public:
    StoryModeGameWidget(int lifes, String aiFace);
    void initWithGUI(GameView &areaA, GameView &areaB, GamePlayer *playercontroller, LevelTheme &levelTheme, int level, Action *gameOverAction);
    virtual ~StoryModeGameWidget();
    bool didPlayerWon() const { return isGameARunning(); }
    void cycle();
    StoryWidget *getOpponent();
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event);
protected:
    int faceTicks;
    StoryWidget opponent;
    CheatCodeManager killLeftCheat, killRightCheat;
};

class StoryModeStandardLayoutGameWidget : public StoryModeGameWidget
{
public:
    StoryModeStandardLayoutGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, int level, int nColors, int lifes, String aiFace, Action *gameOverAction = NULL);
private:
    FloboSetTheme &attachedFloboThemeSet;
    RandomSystem attachedRandom;
    LocalGameFactory attachedGameFactory;
    GameView areaA, areaB;
};

class StoryModeLevelsDefinition {
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
        const SelIA & getAISettings(GameDifficulty difficulty) const {
            switch (difficulty) {
            case EASY:
                return easySettings;
            case MEDIUM:
                return mediumSettings;
            case HARD:
            default:
                return hardSettings;
            }
        }
    };
    StoryModeLevelsDefinition(String levelDefinitionFile);
    LevelDefinition *getLevelDefinition(int levelNumber) { return levelDefinitions[levelNumber]; }
    int getNumLevels() const { return levelDefinitions.size(); }
    virtual ~StoryModeLevelsDefinition();
private:
    void addLevelDefinition(String levelName, String introStory,
			    String opponentStory, String opponentName, String opponent,
                String backgroundTheme, String gameLostStory, String gameOverStory,
			    SelIA easySettings,
			    SelIA mediumSettings, SelIA hardSettings);
    static void end_level(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void get_BoolPreference(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static StoryModeLevelsDefinition *currentDefinition;
    AdvancedBuffer<LevelDefinition *> levelDefinitions;
};

class SharedGameAssets : public StoryNameProvider
{
public:
    std::string    playerName;
    GameDifficulty difficulty;
    GameOptions    gameOptions;
    int            lifes;
    StoryModeLevelsDefinition::LevelDefinition *levelDef;
public:
    virtual std::string getStoryName() const
    {
        return (const char *)(levelDef->gameOverStory);
    }
};

/**
 * Display the animation when the match is over (1P mode)
 */
class StoryModeMatchIsOverState : public GameState, public Action
{
public:
    StoryModeMatchIsOverState(SharedGameAssets *sharedGameAssets,
				 SharedMatchAssets *sharedMatchAssets);
    virtual ~StoryModeMatchIsOverState();
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
class StoryModeMatchState : public GameState,
                               public GameWidgetFactory,
                               public PlayerNameProvider,
                               public Action
{
public:
    StoryModeMatchState(SharedGameAssets *sharedGameAssets);
    // GameState implementation
    virtual void enterState();
    virtual void exitState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // PlayerNameProvider implementation
    virtual String getPlayerName(int playerNumber) const;
    // GameWidgetFactory implementation
    virtual GameWidget *createGameWidget(FloboSetTheme &floboSetTheme,
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
    void setGameLostState(GameState *gameLostState) {
        m_gameLostState = gameLostState;
    }
    void setGameWidgetFactory(GameWidgetFactory *factory) {
        m_gameWidgetFactory = factory;
    }
    SharedMatchAssets *getMatchAssets() {
        return &m_sharedAssets;
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
    GameWidgetFactory *m_gameWidgetFactory;
    GameState *m_nextState;
    GameState *m_abortedState, *m_victoriousState;
    GameState *m_gameLostState, *m_humiliatedState;

    std::auto_ptr<DisplayStoryScreenState> m_introStoryScreen;
    std::auto_ptr<DisplayStoryScreenState> m_opponentStoryScreen;
    std::auto_ptr<SetupMatchState>       m_setupMatch;
    std::auto_ptr<EnterPlayerReadyState> m_enterPlayersReady;
    std::auto_ptr<ExitPlayerReadyState>  m_exitPlayersReady;
    std::auto_ptr<MatchPlayingState>     m_matchPlaying;
    std::auto_ptr<StoryModeMatchIsOverState> m_matchIsOver;
    std::auto_ptr<DisplayStatsState>     m_displayStats;
    std::auto_ptr<CallActionState>       m_abortGame;
    std::auto_ptr<CallActionState>       m_leaveMatch;
};

/**
 * Performs the logic between a won match and the next match
 */
class StoryModePrepareNextMatchState : public GameState
{
public:
    StoryModePrepareNextMatchState(SharedGameAssets *sharedGameAssets);
    // GameState implementation
    virtual void enterState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // Own methods
    void setNextMatchState(GameState *nextMatchState) {
        m_nextMatchState = nextMatchState;
    }
    void setGameWonState(GameState *gameWonState) {
        m_gameWonState = gameWonState;
    }
    void reset();
private:
    static std::auto_ptr<StoryModeLevelsDefinition> m_levelDefProvider;
    SharedGameAssets *m_sharedGameAssets;
    int m_currentLevel;
    GameState *m_nextMatchState, *m_gameWonState;
    GameState *m_nextState;
};



//class StoryModeMatchStateMachine : public GameStateMachine
class StoryModeStarterAction : public Action {
public:
    StoryModeStarterAction(GameDifficulty difficulty,
                           PlayerNameProvider *nameProvider = NULL);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			event_manager::GameControlEvent *event);
protected:
    PlayerNameProvider *m_nameProvider;
    GameStateMachine m_stateMachine;
    SharedGameAssets     m_sharedGameAssets;

    std::auto_ptr<PushScreenState> m_pushGameScreen;
    std::auto_ptr<StoryModePrepareNextMatchState> m_prepareNextMatch;
    std::auto_ptr<StoryModeMatchState>  m_playMatch;
    std::auto_ptr<DisplayStoryScreenState> m_gameWon;
    std::auto_ptr<ManageHiScoresState>     m_gameLostHoF;
    std::auto_ptr<ManageHiScoresState>     m_gameWonHoF;
    std::auto_ptr<LeaveGameState>          m_leaveGame;
};

#endif // _STORYMODESTARTER_H_


