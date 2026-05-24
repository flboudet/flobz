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
    StoryModeGameWidget(int lifes, const std::string & aiFace);
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
    int _faceTicks;
    StoryWidget _opponent;
    CheatCodeManager _killLeftCheat, _killRightCheat;
};

class StoryModeStandardLayoutGameWidget : public StoryModeGameWidget
{
public:
    StoryModeStandardLayoutGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, int level, int nColors, int lifes, const std::string & aiFace, Action *gameOverAction = NULL);
private:
    FloboSetTheme &_attachedFloboThemeSet;
    RandomSystem _attachedRandom;
    LocalGameFactory _attachedGameFactory;
    GameView _areaA, _areaB;
};

class StoryModeLevelsDefinition {
public:
    struct SelIA {
        SelIA(int level, int nColors);
        int _level;
        int _nColors;
    };
    struct LevelDefinition {
        LevelDefinition(const std::string & levelName, const std::string & introStory,
                        const std::string & opponentStory, const std::string & opponentName, const std::string & opponent,
                        const std::string & backgroundTheme, const std::string & gameLostStory, const std::string & gameWonStory, const std::string & gameOverStory,
                        SelIA easySettings, SelIA mediumSettings, SelIA hardSettings)
            : _levelName(levelName), _introStory(introStory),
              _opponentStory(opponentStory),  _opponentName(opponentName), _opponent(opponent),
              _backgroundTheme(backgroundTheme), _gameLostStory(gameLostStory), _gameWonStory(gameWonStory),
              _gameOverStory(gameOverStory),
              _easySettings(easySettings), _mediumSettings(mediumSettings), _hardSettings(hardSettings) {}
        std::string _levelName;
        std::string _introStory;
        std::string _opponentStory;
        std::string _opponentName;
        std::string _opponent;
        std::string _backgroundTheme;
        std::string _gameLostStory;
        std::string _gameWonStory;
        std::string _gameOverStory;
        SelIA _easySettings;
        SelIA _mediumSettings;
        SelIA _hardSettings;
        const SelIA & getAISettings(GameDifficulty difficulty) const {
            switch (difficulty) {
            case EASY:
                return _easySettings;
            case MEDIUM:
                return _mediumSettings;
            case HARD:
            default:
                return _hardSettings;
            }
        }
    };
    StoryModeLevelsDefinition(const std::string &levelDefinitionFile);
    LevelDefinition *getLevelDefinition(int levelNumber) { return _levelDefinitions[levelNumber]; }
    int getNumLevels() const { return _levelDefinitions.size(); }
    virtual ~StoryModeLevelsDefinition();
private:
    void addLevelDefinition(const std::string & levelName, const std::string & introStory,
			    const std::string & opponentStory, const std::string & opponentName, const std::string & opponent,
                const std::string & backgroundTheme, const std::string & gameLostStory, const std::string & gameWonStory, const std::string & gameOverStory,
			    SelIA easySettings,
			    SelIA mediumSettings, SelIA hardSettings);
    static void end_level(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static void get_BoolPreference(GoomSL *gsl, GoomHash *global, GoomHash *local);
    static StoryModeLevelsDefinition *_currentDefinition;
    AdvancedBuffer<LevelDefinition *> _levelDefinitions;
};

class SharedGameAssets : public StoryNameProvider
{
public:
    std::string    _playerName;
    GameDifficulty _difficulty;
    GameOptions    _gameOptions;
    int            _lifes;
    StoryModeLevelsDefinition::LevelDefinition *_levelDef;
public:
    virtual std::string getStoryName() const
    {
        return _levelDef->_gameOverStory;
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
        _nextState = nextState;
    }
private:
    SharedGameAssets *_sharedGameAssets;
    SharedMatchAssets *_sharedMatchAssets;
    std::unique_ptr<StoryWidget> _gameLostWidget;
    bool _aknowledged;
    GameState *_nextState;
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
    virtual std::string getPlayerName(int playerNumber) const;
    // GameWidgetFactory implementation
    virtual GameWidget *createGameWidget(FloboSetTheme &floboSetTheme,
                                         LevelTheme &levelTheme,
                                         const std::string & centerFace,
                                         Action *gameOverAction);
    // Action implementation
    virtual void action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event);
    // Own methods
    void setAbortedState(GameState *abortedState) {
        _abortedState = abortedState;
    }
    void setVictoriousState(GameState *victoriousState) {
        _victoriousState = victoriousState;
    }
    void setHumiliatedState(GameState *humiliatedState) {
        _humiliatedState = humiliatedState;
    }
    void setGameLostState(GameState *gameLostState) {
        _gameLostState = gameLostState;
    }
    void setGameWidgetFactory(GameWidgetFactory *factory) {
        _gameWidgetFactory = factory;
    }
    SharedMatchAssets *getMatchAssets() {
        return &_sharedAssets;
    }
private:
    enum {
        LEAVE_MATCH,
        ABORT_GAME
    };
    GameStateMachine _stateMachine;
    SharedGameAssets *_sharedGameAssets;
    SharedMatchAssets _sharedAssets;
    SharedGetReadyAssets _sharedGetReadyAssets;
    GameWidgetFactory *_gameWidgetFactory;
    GameState *_nextState;
    GameState *_abortedState, *_victoriousState;
    GameState *_gameLostState, *_humiliatedState;

    std::unique_ptr<DisplayStoryScreenState> _introStoryScreen;
    std::unique_ptr<DisplayStoryScreenState> _opponentStoryScreen;
    std::unique_ptr<SetupMatchState>       _setupMatch;
    std::unique_ptr<EnterPlayerReadyState> _enterPlayersReady;
    std::unique_ptr<ExitPlayerReadyState>  _exitPlayersReady;
    std::unique_ptr<MatchPlayingState>     _matchPlaying;
    std::unique_ptr<StoryModeMatchIsOverState> _matchIsOver;
    std::unique_ptr<DisplayStatsState>     _displayStats;
    std::unique_ptr<CallActionState>       _abortGame;
    std::unique_ptr<CallActionState>       _leaveMatch;
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
        _nextMatchState = nextMatchState;
    }
    void setGameWonState(GameState *gameWonState) {
        _gameWonState = gameWonState;
    }
    void reset();
private:
    static std::unique_ptr<StoryModeLevelsDefinition> _levelDefProvider;
    SharedGameAssets *_sharedGameAssets;
    int _currentLevel;
    GameState *_nextMatchState, *_gameWonState;
    GameState *_nextState;
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
    // Own methods
    void pauseGameIfPossible();
protected:
    PlayerNameProvider *_nameProvider;
    GameStateMachine _stateMachine;
    SharedGameAssets     _sharedGameAssets;

    std::unique_ptr<PushScreenState> _pushGameScreen;
    std::unique_ptr<StoryModePrepareNextMatchState> _prepareNextMatch;
    std::unique_ptr<StoryModeMatchState>  _playMatch;
    std::unique_ptr<DisplayStoryScreenState> _gameWon;
    std::unique_ptr<ManageHiScoresState>     _gameLostHoF;
    std::unique_ptr<ManageHiScoresState>     _gameWonHoF;
    std::unique_ptr<LeaveGameState>          _leaveGame;
};

#endif // _STORYMODESTARTER_H_


