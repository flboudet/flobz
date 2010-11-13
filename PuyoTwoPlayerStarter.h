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
#ifndef _PUYOTWOPLAYERSTARTER
#define _PUYOTWOPLAYERSTARTER

#include "GameStateMachine.h"
#include "PuyoStarter.h"
#include "StatsWidget.h"

class PuyoTwoNameProvider {
public:
    virtual String getPlayer1Name() const = 0;
    virtual String getPlayer2Name() const = 0;
    virtual ~PuyoTwoNameProvider() {};
};

class TwoPlayersGameWidget : public GameWidget {
public:
    TwoPlayersGameWidget(PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme, String aiFace, Action *gameOverAction = NULL);
    bool didPlayerWon() const { return isGameARunning(); }
    void cycle();
    StoryWidget *getOpponent();
private:
    PuyoSetTheme &attachedPuyoThemeSet;
    PuyoRandomSystem attachedRandom;
    PuyoLocalGameFactory attachedGameFactory;
    PuyoView areaA, areaB;
    PuyoEventPlayer playercontrollerA, playercontrollerB;
    StoryWidget opponentFace;
};


class GameWidgetFactory {
public:
    virtual GameWidget *createGameWidget(PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme, String centerFace, Action *gameOverAction) = 0;
    virtual ~GameWidgetFactory() {};
};

class PuyoLocalTwoPlayerGameWidgetFactory : public GameWidgetFactory {
public:
    GameWidget *createGameWidget(PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme, String centerFace, Action *gameOverAction)
    {
        return new TwoPlayersGameWidget(puyoThemeSet, levelTheme, centerFace, gameOverAction);
    }
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

class SetupMatchState : public GameState, public Action
{
public:
    SetupMatchState(GameWidgetFactory &gameWidgetFactory,
                    int difficulty,
                    PuyoTwoNameProvider *nameProvider,
                    SharedMatchAssets &sharedMatchAssets);
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
    GameWidgetFactory &m_gameWidgetFactory;
    int m_difficulty;
    PuyoTwoNameProvider *m_nameProvider;
    SharedMatchAssets &m_sharedAssets;
    GameState *m_nextState;
};

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

class MatchIsOverState : public GameState, public Action
{
public:
    MatchIsOverState(SharedMatchAssets &sharedMatchAssets);
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

class DisplayStatsState : public GameState, public Action
{
public:
    DisplayStatsState(SharedMatchAssets &sharedMatchAssets);
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
    auto_ptr<TwoPlayersStatsWidget> m_statsWidget;
    GameState *m_nextState;
};

class LeaveGameState : public GameState
{
public:
    LeaveGameState(SharedMatchAssets &sharedMatchAssets);
    // GameState implementation
    virtual void enterState();
    virtual bool evaluate();
    virtual GameState *getNextState();
private:
    SharedMatchAssets &m_sharedAssets;
};



class AltTwoPlayersStarterAction : public Action {
public:
    AltTwoPlayersStarterAction(int difficulty, GameWidgetFactory &gameWidgetFactory, PuyoTwoNameProvider *nameProvider = NULL);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			event_manager::GameControlEvent *event);
private:
    GameStateMachine m_stateMachine;
    SharedMatchAssets m_sharedAssets;
    auto_ptr<SetupMatchState> m_setupMatch;
    auto_ptr<WaitPlayersReadyState> m_waitPlayersReady;
    auto_ptr<MatchPlayingState>     m_matchPlaying;
    auto_ptr<MatchIsOverState>      m_matchIsOver;
    auto_ptr<DisplayStatsState>     m_displayStats;
    auto_ptr<LeaveGameState>       m_leaveGame;
};

class TwoPlayersStarterAction : public Action {
public:
    TwoPlayersStarterAction(int difficulty, GameWidgetFactory &gameWidgetFactory, PuyoTwoNameProvider *nameProvider = NULL);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			event_manager::GameControlEvent *event);
private:
    enum State {
      kNotRunning,
      kMatchGettingStarted,
      kMatchPlaying,
      kMatchWonP1Animation,
      kMatchWonP2Animation,
      kMatchScores,
    };
    /**
     * Performs a step in the match state machine
     */
    void stateMachine();
    void prepareGame();
    void prepareGame1stRun();
    void prepareGameNextRun();
    void startGame();
    void gameOver();
    void gameScores();
    void endGameSession();
    State m_state;
    int difficulty;
    GameWidgetFactory &gameWidgetFactory;
    GameScreen *gameScreen;
    GameWidget *gameWidget;
    PuyoTwoNameProvider *nameProvider;
    StoryWidget *gameLostWidget;
    StoryWidget *m_getReadyWidget;

    LevelTheme *currentLevelTheme;
    int leftVictories, rightVictories;
    TwoPlayersStatsWidget *m_statsWidget;
    int totalPointsPlayerOne_, totalPointsPlayerTwo_;
};

#endif // _PUYOTWOPLAYERSTARTER
