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

#include "PuyoTwoPlayerStarter.h"
using namespace event_manager;

TwoPlayersGameWidget::TwoPlayersGameWidget(PuyoSetTheme &puyoThemeSet, LevelTheme &levelTheme, String aiFace, Action *gameOverAction) : attachedPuyoThemeSet(puyoThemeSet),
                                                     attachedRandom(5), attachedGameFactory(&attachedRandom),
                                                     areaA(&attachedGameFactory, 0, &attachedPuyoThemeSet, &levelTheme),
                                                     areaB(&attachedGameFactory, 1, &attachedPuyoThemeSet, &levelTheme),
                                                     playercontrollerA(areaA, kPlayer1Down, kPlayer1Left, kPlayer1Right,
                                                     kPlayer1TurnLeft, kPlayer1TurnRight),
                                                     playercontrollerB(areaB, kPlayer2Down, kPlayer2Left, kPlayer2Right,
                                                     kPlayer2TurnLeft, kPlayer2TurnRight),
                                                     opponentFace(aiFace)
{
    initWithGUI(areaA, areaB, playercontrollerA, playercontrollerB, levelTheme, gameOverAction);
    setLives(-1);
}

StoryWidget *TwoPlayersGameWidget::getOpponent()
{
    return &opponentFace;
}

void TwoPlayersGameWidget::cycle()
{
    opponentFace.setIntegerValue("@maxHeightLeft", attachedGameA->getColumnHeigth(2));
    opponentFace.setIntegerValue("@maxHeightRight", attachedGameB->getColumnHeigth(2));
    opponentFace.setIntegerValue("@neutralsForLeft", attachedGameA->getNeutralPuyos());
    opponentFace.setIntegerValue("@neutralsForRight", attachedGameB->getNeutralPuyos());
    opponentFace.setIntegerValue("@comboPhaseLeft", attachedGameA->getComboPhase());
    opponentFace.setIntegerValue("@comboPhaseRight", attachedGameB->getComboPhase());
    GameWidget::cycle();
}

//---------------------------------
// Two players local game state machine
//---------------------------------
AltTwoPlayersStarterAction::AltTwoPlayersStarterAction(GameDifficulty difficulty, GameWidgetFactory *gameWidgetFactory, PlayerNameProvider *nameProvider)
{
    // Creating the different game states
    m_pushGameScreen.reset(new PushScreenState());
    m_setupMatch.reset(new SetupMatchState(gameWidgetFactory, GameOptions::fromDifficulty(difficulty), nameProvider, &m_sharedAssets));
    m_enterPlayersReady.reset(new EnterPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    m_exitPlayersReady.reset(new ExitPlayerReadyState(m_sharedAssets, m_sharedGetReadyAssets));
    m_matchPlaying.reset(new MatchPlayingState(m_sharedAssets));
    m_matchIsOver.reset(new MatchIsOverState(m_sharedAssets));
    m_displayStats.reset(new DisplayStatsState(m_sharedAssets));
    m_leaveGame.reset(new LeaveGameState(m_sharedAssets));
    // Linking the states together
    m_pushGameScreen->setNextState(m_setupMatch.get());
    m_setupMatch->setNextState(m_enterPlayersReady.get());
    m_enterPlayersReady->setNextState(m_exitPlayersReady.get());
    m_exitPlayersReady->setNextState(m_matchPlaying.get());
    m_matchPlaying->setNextState(m_matchIsOver.get());
    m_matchPlaying->setAbortedState(m_leaveGame.get());
    m_matchIsOver->setNextState(m_displayStats.get());
    m_displayStats->setNextState(m_setupMatch.get());
    // Initializing the state machine
    m_stateMachine.setInitialState(m_pushGameScreen.get());
}

void AltTwoPlayersStarterAction::action(Widget *sender, int actionType,
                                        event_manager::GameControlEvent *event)
{
    m_stateMachine.reset();
    m_stateMachine.evaluate();
}

