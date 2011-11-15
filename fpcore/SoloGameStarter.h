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
#ifndef _SOLOGAMESTARTER_H_
#define _SOLOGAMESTARTER_H_

#include "GameStateMachine.h"
#include "LocalGameStates.h"
#include "EventPlayer.h"

class SoloGameWidget : public GameWidget, GameListener, CycledComponent {
public:
    SoloGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, Action *gameOverAction = NULL);
    // GameWidget implementation
    virtual void setGameOptions(GameOptions options);
    //
    virtual void pause(bool obscureScreen = true);
    virtual void resume();
    // Callbacks
    virtual bool backPressed();
    virtual bool startPressed();
    virtual void abort();
    virtual bool getAborted() const;
    //
    virtual StoryWidget *getOpponent();
    // TODO: Make this N-players generic (N from 1 to +inf)
    virtual void setPlayerOneName(String newName);
    virtual void setPlayerTwoName(String newName);
    virtual PlayerGameStat &getStatPlayerOne();
    virtual PlayerGameStat &getStatPlayerTwo();
    virtual void addGameAHandicap(int handicap);
    virtual void addGameBHandicap(int handicap);
    virtual bool isGameARunning() const;
    // GameListener implementation
    virtual void gameDidEndCycle();
    virtual void floboWillVanish(AdvancedBuffer<Flobo *> &floboGroup, int groupNum, int phase);
    // CycledComponent implementation
    virtual void cycle();
    // Widget methods
    void draw(DrawTarget *dt);
    bool isFocusable() { return !paused; }
    IdleComponent *getIdleComponent() { return this; }
    void eventOccured(event_manager::GameControlEvent *event);
private:
    FloboSetTheme &attachedFloboThemeSet;
    LevelTheme &attachedLevelTheme;
    RandomSystem attachedRandom;
    std::auto_ptr<LocalGameFactory> m_gameFactory;
    std::auto_ptr<GameView>        m_areaA;
    std::auto_ptr<CombinedEventPlayer> m_playerController;
    PlayerGameStat m_gameStat;
    GameOptions m_options;
    int m_cyclesBeforeGameCycle;
    int m_cyclesBeforeLevelRaise;
};

class SoloGameWidgetFactory : public GameWidgetFactory {
public:
    GameWidget *createGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, String centerFace, Action *gameOverAction)
    {
        return new SoloGameWidget(floboSetTheme, levelTheme, gameOverAction);
    }
};

class SoloModeStarterAction : public Action {
public:
    SoloModeStarterAction(GameDifficulty difficulty,
                          PlayerNameProvider *nameProvider = NULL);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			event_manager::GameControlEvent *event);
private:
    SoloGameWidgetFactory m_gameWidgetFactory;
    GameStateMachine m_stateMachine;
    SharedMatchAssets m_sharedAssets;
    SharedGetReadyAssets        m_sharedGetReadyAssets;
    std::auto_ptr<PushScreenState>       m_pushGameScreen;
    std::auto_ptr<SetupMatchState>       m_setupMatch;
    std::auto_ptr<EnterPlayerReadyState> m_enterPlayersReady;
    std::auto_ptr<ExitPlayerReadyState>  m_exitPlayersReady;
    std::auto_ptr<MatchPlayingState>     m_matchPlaying;
    std::auto_ptr<MatchIsOverState>      m_matchIsOver;
    std::auto_ptr<LeaveGameState>        m_leaveGame;
};

#endif // _SOLOGAMESTARTER_H_
