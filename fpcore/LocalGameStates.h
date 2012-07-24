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
    virtual String getPlayerName(int playerNumber) const = 0;
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
                                         String centerFace,
                                         Action *gameOverAction) = 0;
    virtual ~GameWidgetFactory() {};
};

struct SharedMatchAssets
{
    SharedMatchAssets() {
        release();
    }
    FloboSetThemeRef m_currentFloboSetTheme;
    LevelThemeRef m_currentLevelTheme;
    std::auto_ptr<GameWidget> m_gameWidget;
    std::auto_ptr<GameScreen> m_gameScreen;
    int m_leftVictories, m_leftTotal;
    int m_rightVictories, m_rightTotal;
    void release() {
        m_currentFloboSetTheme.release();
        m_currentLevelTheme.release();
        m_gameWidget.reset(NULL);
        m_gameScreen.reset(NULL);
        m_leftVictories = 0;
        m_rightVictories = 0;
        m_leftTotal = 0;
        m_rightTotal = 0;
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
        m_nextState = nextState;
    }
private:
    class GhostScreen;
    std::auto_ptr<GhostScreen> m_ghostScreen;
    GameState *m_nextState;
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
        m_nextState = nextState;
    }
    void setHandicapOnVictorious(bool enable) {
        m_handicapOnVictorious = enable;
    }
    void setDisplayVictories(bool enable) {
        m_displayVictories = enable;
    }
    void setAccountTotalOnPlayerB(bool enable) {
        m_accountTotalOnPlayerB = enable;
    }
    void setGameWidgetFactory(GameWidgetFactory *factory) {
        m_gameWidgetFactory = factory;
    }
private:
    int m_nbPlayers;
    GameWidgetFactory  *m_gameWidgetFactory;
    GameOptions         m_gameOptions;
    PlayerNameProvider *m_nameProvider;
    SharedMatchAssets  *m_sharedAssets;
    GameState *m_nextState;
    bool m_handicapOnVictorious;
    bool m_displayVictories;
    bool m_accountTotalOnPlayerB;
};
struct SharedGetReadyAssets
{
    auto_ptr<StoryWidget> m_getReadyWidget;
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
        m_nextState = nextState;
    }
private:
    SharedMatchAssets &m_sharedAssets;
    SharedGetReadyAssets &m_sharedGetReadyAssets;
    bool m_getReadyDisplayed;
    GameState *m_nextState;
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
        m_nextState = nextState;
    }
private:
    SharedMatchAssets &m_sharedAssets;
    SharedGetReadyAssets &m_sharedGetReadyAssets;
    GameState *m_nextState;
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
        m_nextState = nextState;
    }
private:
    SharedMatchAssets &m_sharedAssets;
    bool m_playersAreReady;
    auto_ptr<StoryWidget> m_getReadyWidget;
    GameState *m_nextState;
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
        m_nextState = nextState;
    }
    void setAbortedState(GameState *abortedState) {
        m_abortedState = abortedState;
    }
private:
    SharedMatchAssets &m_sharedAssets;
    bool m_gameIsOver;
    GameState *m_nextState, *m_abortedState;
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
        m_nextState = nextState;
    }
private:
    SharedMatchAssets &m_sharedAssets;
    std::auto_ptr<StoryWidget> m_gameLostWidget;
    bool m_aknowledged;
    GameState *m_nextState;
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
        m_nextState = nextState;
    }
private:
    SharedMatchAssets &m_sharedAssets;
    bool m_aknowledged;
    StatsWidgetDimensions m_dimensions;
    auto_ptr<TwoPlayersStatsWidget> m_statsWidget;
    GameState *m_nextState;
};

/**
 * Display a story screen, and wait for the user to acknowledge
 * or the story to end
 */
class DisplayStoryScreenState : public GameState, public Action
{
public:
    DisplayStoryScreenState(const char *screenName);
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
    std::string m_screenName;
    bool m_acknowledged;
    std::auto_ptr<StoryScreen> m_storyScreen;
    GameState *m_nextState;
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
        m_nextState = nextState;
    }
    void setHiScoreBoard(HiScoreBoard *board) {
        m_scoreBoard = board;
    }
    void setFinalScore(const char *playerName, int score) {
        m_playerName = playerName;
        m_playerScore = score;
    }
    void setRank(int rank) {
        m_rank = rank;
    }
    // Action implementation
    virtual void action(Widget *sender, int actionType,
                        event_manager::GameControlEvent *event);
private:
    std::string m_storyName;
    StoryNameProvider  *m_storyNameProvider;
    GameState *m_nextState;
    std::auto_ptr<GameOverScreen> m_gameOverScreen;
    HiScoreBoard *m_scoreBoard;
    std::string m_playerName;
    int m_playerScore;
    int m_rank;
    bool m_acknowledged;
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
    SharedMatchAssets &m_sharedAssets;
    Action *m_actionToCallWhenLeft;
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
    Action *m_actionToCall;
    int     m_actionType;
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
        m_nextState = nextState;
    }
private:
    std::string m_boardId;
    SharedMatchAssets  *m_sharedMatchAssets;
    PlayerNameProvider *m_nameProvider;
    bool m_finished;

    GameState *m_nextState;
    std::auto_ptr<DisplayStoryScreenState> m_newHiScore;
    std::auto_ptr<DisplayHallOfFameState> m_displayHallOfFame;
    std::auto_ptr<CallActionState> m_endOfStateMachine;
    GameStateMachine m_stateMachine;
    std::auto_ptr<LocalStorageHiScoreBoard> m_scoreBoard;
    HiScoreDefaultBoard m_defaultScoreBoard;
};

/**
 * Manage multi-sets in a game
 */
class ManageMultiSetsState : public GameState
{
public:
    ManageMultiSetsState(SharedMatchAssets  *sharedMatchAssets, int nbSets);
    // GameState implementation
    virtual void enterState();
    virtual void exitState();
    virtual bool evaluate();
    virtual GameState *getNextState();
    // Own methods
    void setNextSetState(GameState *nextSetState) {
        m_nextSetState = nextSetState;
    }
    void setEndOfGameState(GameState *endOfGameState) {
        m_endOfGameState = endOfGameState;
    }
private:
    SharedMatchAssets  *m_sharedAssets;
    int m_nbSets;
    GameState *m_nextSetState;
    GameState *m_endOfGameState;
};


#endif // _LOCALGAMESTATES_H

