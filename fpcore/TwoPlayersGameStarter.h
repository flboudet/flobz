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
#ifndef _TWOPLAYERSGAMESTARTER_H_
#define _TWOPLAYERSGAMESTARTER_H_

#include "GameStateMachine.h"
#include "LocalGameStates.h"
#include "EventPlayer.h"

class TwoPlayersGameWidget : public GameWidget2P {
public:
    TwoPlayersGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, String aiFace, Action *gameOverAction = NULL);
    bool didPlayerWon() const { return isGameARunning(); }
    void cycle();
    StoryWidget *getOpponent();
private:
    FloboSetTheme &attachedFloboThemeSet;
    RandomSystem attachedRandom;
    LocalGameFactory attachedGameFactory;
    GameView areaA, areaB;
    EventPlayer playercontrollerA, playercontrollerB;
    StoryWidget opponentFace;
};

class LocalTwoPlayersGameWidgetFactory : public GameWidgetFactory {
public:
    GameWidget *createGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, String centerFace, Action *gameOverAction)
    {
        return new TwoPlayersGameWidget(floboSetTheme, levelTheme, centerFace, gameOverAction);
    }
};

class AltTwoPlayersStarterAction : public Action {
public:
    AltTwoPlayersStarterAction(GameDifficulty difficulty, GameWidgetFactory *gameWidgetFactory,
                               PlayerNameProvider *nameProvider = NULL);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			event_manager::GameControlEvent *event);
private:
    GameStateMachine m_stateMachine;
    SharedMatchAssets m_sharedAssets;
    SharedGetReadyAssets        m_sharedGetReadyAssets;
    std::auto_ptr<PushScreenState>       m_pushGameScreen;
    std::auto_ptr<SetupMatchState>       m_setupMatch;
    std::auto_ptr<EnterPlayerReadyState> m_enterPlayersReady;
    std::auto_ptr<ExitPlayerReadyState>  m_exitPlayersReady;
    std::auto_ptr<MatchPlayingState>     m_matchPlaying;
    std::auto_ptr<MatchIsOverState>      m_matchIsOver;
    std::auto_ptr<DisplayStatsState>     m_displayStats;
    std::auto_ptr<LeaveGameState>        m_leaveGame;
};

#endif // _TWOPLAYERSGAMESTARTER_H_
