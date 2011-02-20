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

#ifndef _LOCALGAMESTATES_H
#define _LOCALGAMESTATES_H

#include "GameStateMachine.h"
#include "GameWidget.h"
#include "GameScreen.h"
#include "StatsWidget.h"

class PlayerNameProvider {
public:
    virtual String getPlayerName(int playerNumber) const = 0;
    virtual ~PlayerNameProvider() {};
};

class GameWidgetFactory {
public:
    virtual GameWidget *createGameWidget(PuyoSetTheme &puyoThemeSet,
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
    PuyoSetThemeRef m_currentPuyoSetTheme;
    LevelThemeRef m_currentLevelTheme;
    std::auto_ptr<GameWidget> m_gameWidget;
    std::auto_ptr<GameScreen> m_gameScreen;
    int m_leftVictories, m_leftTotal;
    int m_rightVictories, m_rightTotal;
    void release() {
        m_currentPuyoSetTheme.release();
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
                    SharedMatchAssets *sharedMatchAssets);
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
    void setAccountTotalOnPlayerB(bool enable) {
        m_accountTotalOnPlayerB = enable;
    }
private:
    GameWidgetFactory  *m_gameWidgetFactory;
    GameOptions         m_gameOptions;
    PlayerNameProvider *m_nameProvider;
    SharedMatchAssets  *m_sharedAssets;
    GameState *m_nextState;
    bool m_handicapOnVictorious;
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

#endif // _LOCALGAMESTATES_H

