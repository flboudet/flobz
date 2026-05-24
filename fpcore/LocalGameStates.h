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

#ifndef _LOCALGAMESTATES_H
#define _LOCALGAMESTATES_H

#include "GameStateMachine.h"
#include "GameWidget.h"
#include "GameScreen.h"
#include "StatsWidget.h"
#include "HallOfFame.h"

class PlayerNameProvider {
public:
    virtual std::string getPlayerName(int playerNumber) const = 0;
    virtual ~PlayerNameProvider() {};
};

class StoryNameProvider {
public:
    virtual std::string getStoryName() const = 0;
    virtual ~StoryNameProvider() {}
};

class GameWidgetFactory {
public:
    virtual GameWidget *createGameWidget(FloboSetTheme &floboSetTheme,
                                         LevelTheme &levelTheme,
                                         const std::string & centerFace,
                                         Action *gameOverAction) = 0;
    virtual ~GameWidgetFactory() {};
};

struct SharedMatchAssets
{
    SharedMatchAssets() {
        release();
    }
    FloboSetThemeRef _currentFloboSetTheme;
    LevelThemeRef _currentLevelTheme;
    std::unique_ptr<GameWidget> _gameWidget;
    std::unique_ptr<GameScreen> _gameScreen;
    int _leftVictories, _leftTotal;
    int _rightVictories, _rightTotal;
    void release() {
        _currentFloboSetTheme.release();
        _currentLevelTheme.release();
        _gameWidget.reset(NULL);
        _gameScreen.reset(NULL);
        _leftVictories = 0;
        _rightVictories = 0;
        _leftTotal = 0;
        _rightTotal = 0;
    }
};

/**
 * Short: Always call this as the first state of a game.
 * Pushes a dummy screen on the screen stack.
 * All further states that manipulate the screen stack
 * will normally swap with the topmost screen, so we need
 * to put a dummy screen at the top before the first swap
 * (otherwise the menu screen would be swapped)
 */
class PushScreenState : public GameState
{
public:
    // GameState implementation
    virtual void enterState();
    virtual void exitState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // Own methods
    void setNextState(GameState *nextState) {
        _nextState = nextState;
    }
private:
    class GhostScreen;
    std::unique_ptr<gameui::Screen> _ghostScreen;
    GameState *_nextState;
};

/**
 * Setups the match screen, ready for a new game
 */
class SetupMatchState : public GameState, public Action
{
public:
    SetupMatchState(GameWidgetFactory *gameWidgetFactory,
                    GameOptions gameOptions,
                    PlayerNameProvider *nameProvider,
                    SharedMatchAssets *sharedMatchAssets,
                    int nbPlayers = 2);
    // GameState implementation
    virtual void enterState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // Action implementation
    virtual void action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event);
    // Own methods
    void setNextState(GameState *nextState) {
        _nextState = nextState;
    }
    void setHandicapOnVictorious(bool enable) {
        _handicapOnVictorious = enable;
    }
    void setDisplayVictories(bool enable) {
        _displayVictories = enable;
    }
    void setAccountTotalOnPlayerB(bool enable) {
        _accountTotalOnPlayerB = enable;
    }
    void setGameWidgetFactory(GameWidgetFactory *factory) {
        _gameWidgetFactory = factory;
    }
private:
    int _nbPlayers;
    GameWidgetFactory  *_gameWidgetFactory;
    GameOptions         _gameOptions;
    PlayerNameProvider *_nameProvider;
    SharedMatchAssets  *_sharedAssets;
    GameState *_nextState;
    bool _handicapOnVictorious;
    bool _displayVictories;
    bool _accountTotalOnPlayerB;
};
struct SharedGetReadyAssets
{
    unique_ptr<StoryWidget> _getReadyWidget;
};

/**
 * Begins the "get ready" animation
 */
class EnterPlayerReadyState : public GameState, public Action, CycledComponent
{
public:
    EnterPlayerReadyState(SharedMatchAssets &sharedMatchAssets,
                          SharedGetReadyAssets &sharedGetReadyAssets);
    // GameState implementation
    virtual void enterState();
    virtual void exitState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // Action implementation
    virtual void action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event);
    // CycledComponent implementation
    virtual void cycle();
    virtual void onEvent(event_manager::GameControlEvent *cevent);
    // Own methods
    void setNextState(GameState *nextState) {
        _nextState = nextState;
    }
private:
    SharedMatchAssets &_sharedAssets;
    SharedGetReadyAssets &_sharedGetReadyAssets;
    bool _getReadyDisplayed;
    GameState *_nextState;
};

/**
 * Ends the "get ready" animation
 */
class ExitPlayerReadyState : public GameState
{
public:
    ExitPlayerReadyState(SharedMatchAssets &sharedMatchAssets,
                         SharedGetReadyAssets &sharedGetReadyAssets);
    // GameState implementation
    virtual void enterState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // Own methods
    void setNextState(GameState *nextState) {
        _nextState = nextState;
    }
private:
    SharedMatchAssets &_sharedAssets;
    SharedGetReadyAssets &_sharedGetReadyAssets;
    GameState *_nextState;
};

/**
 * DEPRECATED: plays the "get ready" animation
 */
class WaitPlayersReadyState : public GameState, public Action
{
public:
    WaitPlayersReadyState(SharedMatchAssets &sharedMatchAssets);
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
    SharedMatchAssets &_sharedAssets;
    bool _playersAreReady;
    unique_ptr<StoryWidget> _getReadyWidget;
    GameState *_nextState;
};

/**
 * Make the match play until it is finished or aborted
 */
class MatchPlayingState : public GameState, public Action
{
public:
    MatchPlayingState(SharedMatchAssets &sharedMatchAssets);
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
    void setAbortedState(GameState *abortedState) {
        _abortedState = abortedState;
    }
private:
    SharedMatchAssets &_sharedAssets;
    bool _gameIsOver;
    GameState *_nextState, *_abortedState;
};

/**
 * Display the animation when the match is over
 */
class MatchIsOverState : public GameState, public Action
{
public:
    MatchIsOverState(SharedMatchAssets &sharedMatchAssets);
    virtual ~MatchIsOverState();
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
    void setStyrolyse(std::string styrolyse) { _styrolyseName = styrolyse; }
private:
    SharedMatchAssets &_sharedAssets;
    std::string _styrolyseName;
    std::unique_ptr<StoryWidget> _gameLostWidget;
    bool _aknowledged;
    GameState *_nextState;
};

/**
 * Display the statistics of the game
 */
class DisplayStatsState : public GameState, public Action
{
public:
    DisplayStatsState(SharedMatchAssets &sharedMatchAssets);
    virtual ~DisplayStatsState();
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
    SharedMatchAssets &_sharedAssets;
    bool _aknowledged;
    StatsWidgetDimensions _dimensions;
    unique_ptr<TwoPlayersStatsWidget> _statsWidget;
    GameState *_nextState;
};

/**
 * Provides variable values to a story from a DisplayStoryScreenState
 */
class StoryScreenValuesProvider
{
public:
    virtual std::map<std::string, int>   getIntValues() const = 0;
    virtual std::map<std::string, float> getFloatValues() const = 0;
    virtual std::map<std::string, std::string> getStringValues() const = 0;
};

/**
 * Display a story screen, and wait for the user to acknowledge
 * or the story to end
 */
class DisplayStoryScreenState : public GameState, public Action
{
public:
    DisplayStoryScreenState(const std::string &screenName);
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
    void setStoryScreenValuesProvider(StoryScreenValuesProvider *vp) {
        _vp = vp;
    }
private:
    std::string _screenName;
    bool _acknowledged;
    std::unique_ptr<StoryScreen> _storyScreen;
    GameState *_nextState;
    StoryScreenValuesProvider *_vp;
};

/**
 * Show the Hall of Fame
 */
class DisplayHallOfFameState : public GameState, public Action
{
public:
    DisplayHallOfFameState(const char         *storyName="",
                           StoryNameProvider  *storyNameProvider = NULL);
    // GameState implementation
    virtual void enterState();
    virtual void exitState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // Own methods
    void setNextState(GameState *nextState) {
        _nextState = nextState;
    }
    void setHiScoreBoard(HiScoreBoard *board) {
        _scoreBoard = board;
    }
    void setFinalScore(const char *playerName, int score) {
        _playerName = playerName;
        _playerScore = score;
    }
    void setRank(int rank) {
        _rank = rank;
    }
    // Action implementation
    virtual void action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event);
private:
    std::string _storyName;
    StoryNameProvider  *_storyNameProvider;
    GameState *_nextState;
    std::unique_ptr<GameOverScreen> _gameOverScreen;
    HiScoreBoard *_scoreBoard;
    std::string _playerName;
    int _playerScore;
    int _rank;
    bool _acknowledged;
};

/**
 * Leave the game and rewind to the previous screen
 * TODO: make this state deprecated
 */
class LeaveGameState : public GameState
{
public:
    LeaveGameState(SharedMatchAssets &sharedMatchAssets,
                   Action *actionToCallWhenLeft = NULL);
    // GameState implementation
    virtual void enterState();
    virtual bool evaluate();
    virtual GameState *getNextState();
private:
    SharedMatchAssets &_sharedAssets;
    Action *_actionToCallWhenLeft;
};

/**
 * Call an action on entering the state
 * (useful to mark the end of a sub- state machine)
 */
class CallActionState : public GameState
{
public:
    CallActionState(Action *actionToCall, int actionType);
    // GameState implementation
    virtual void enterState();
    virtual bool evaluate();
    virtual GameState *getNextState();
private:
    Action *_actionToCall;
    int     _actionType;
};

/**
 * Manage Hi Scores, provide new hi score animation when necessary
 */
class ManageHiScoresState : public GameState, public Action
{
public:
    ManageHiScoresState(SharedMatchAssets  *sharedMatchAssets,
                        PlayerNameProvider *nameProvider,
                        const char         *scoreBoardId,
                        const char         *storyName="",
                        StoryNameProvider  *storyNameProvider = NULL);
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
    std::string _boardId;
    SharedMatchAssets  *_sharedMatchAssets;
    PlayerNameProvider *_nameProvider;
    bool _finished;

    GameState *_nextState;
    std::unique_ptr<DisplayStoryScreenState> _newHiScore;
    std::unique_ptr<DisplayHallOfFameState> _displayHallOfFame;
    std::unique_ptr<CallActionState> _endOfStateMachine;
    GameStateMachine _stateMachine;
    std::unique_ptr<LocalStorageHiScoreBoard> _scoreBoard;
    HiScoreDefaultBoard _defaultScoreBoard;
};

/**
 * Manage multi-sets in a game
 */
class ManageMultiSetsState : public GameState, public StoryScreenValuesProvider
{
public:
    ManageMultiSetsState(SharedMatchAssets  *sharedMatchAssets, int nbSets, PlayerNameProvider *nameProvider);
    // GameState implementation
    virtual void enterState();
    virtual void exitState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // StoryScreenValuesProvider implementation
    virtual std::map<std::string, int> getIntValues() const;
    virtual std::map<std::string, float> getFloatValues() const;
    virtual std::map<std::string, std::string> getStringValues() const;
    // Own methods
    void setNextSetState(GameState *nextSetState) {
        _nextSetState = nextSetState;
    }
    void setEndOfGameState(GameState *endOfGameState) {
        _endOfGameState = endOfGameState;
    }
private:
    SharedMatchAssets  *_sharedAssets;
    PlayerNameProvider *_nameProvider;
    int _nbSets;
    GameState *_nextSetState;
    GameState *_endOfGameState;
};


#endif // _LOCALGAMESTATES_H
