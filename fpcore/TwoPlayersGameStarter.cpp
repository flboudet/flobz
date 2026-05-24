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

#include "TwoPlayersGameStarter.h"
using namespace event_manager;

TwoPlayersGameWidget::TwoPlayersGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, const std::string & aiFace, Action *gameOverAction) : _attachedFloboThemeSet(floboSetTheme),
                                                     _attachedRandom(5), _attachedGameFactory(&_attachedRandom),
                                                     _areaA(&_attachedGameFactory, 0, &_attachedFloboThemeSet, &levelTheme),
                                                     _areaB(&_attachedGameFactory, 1, &_attachedFloboThemeSet, &levelTheme),
                                                     _opponentFace(aiFace)
{
    _controllerA.reset(new EventPlayer(_areaA, kPlayer1Down, kPlayer1Left, kPlayer1Right,
                                      kPlayer1TurnLeft, kPlayer1TurnRight));
    _controllerB.reset(new EventPlayer(_areaB, kPlayer2Down, kPlayer2Left, kPlayer2Right,
                                      kPlayer2TurnLeft, kPlayer2TurnRight));
    initWithGUI(_areaA, _areaB, levelTheme, gameOverAction);
    setLives(-1);
}

StoryWidget *TwoPlayersGameWidget::getOpponent()
{
    return &_opponentFace;
}

void TwoPlayersGameWidget::cycle()
{
    _opponentFace.setIntegerValue("@maxHeightLeft", _attachedGameA->getColumnHeigth(2));
    _opponentFace.setIntegerValue("@maxHeightRight", _attachedGameB->getColumnHeigth(2));
    _opponentFace.setIntegerValue("@neutralsForLeft", _attachedGameA->getNeutralFlobos());
    _opponentFace.setIntegerValue("@neutralsForRight", _attachedGameB->getNeutralFlobos());
    _opponentFace.setIntegerValue("@comboPhaseLeft", _attachedGameA->getComboPhase());
    _opponentFace.setIntegerValue("@comboPhaseRight", _attachedGameB->getComboPhase());
    GameWidget2P::cycle();
}

//---------------------------------
// Two players local game state machine
//---------------------------------
AltTwoPlayersStarterAction::AltTwoPlayersStarterAction(GameDifficulty difficulty, GameWidgetFactory *gameWidgetFactory, PlayerNameProvider *nameProvider, int nbSets)
{
    // Creating the different game states
    _pushGameScreen.reset(new PushScreenState());
    _setupMatch.reset(new SetupMatchState(gameWidgetFactory, GameOptions::fromDifficulty(difficulty), nameProvider, &_sharedAssets));
    _enterPlayersReady.reset(new EnterPlayerReadyState(_sharedAssets, _sharedGetReadyAssets));
    _exitPlayersReady.reset(new ExitPlayerReadyState(_sharedAssets, _sharedGetReadyAssets));
    _matchPlaying.reset(new MatchPlayingState(_sharedAssets));
    _matchIsOver.reset(new MatchIsOverState(_sharedAssets));
    _displayStats.reset(new DisplayStatsState(_sharedAssets));
    _manageMultiSets.reset(new ManageMultiSetsState(&_sharedAssets, nbSets, nameProvider));
    _podium.reset(new DisplayStoryScreenState("end_of_multiset.gsl"));
    _leaveGame.reset(new LeaveGameState(_sharedAssets));
    // Linking the states together
    _pushGameScreen->setNextState(_setupMatch.get());
    _setupMatch->setNextState(_enterPlayersReady.get());
    _enterPlayersReady->setNextState(_exitPlayersReady.get());
    _exitPlayersReady->setNextState(_matchPlaying.get());
    _matchPlaying->setNextState(_matchIsOver.get());
    _matchPlaying->setAbortedState(_leaveGame.get());
    _matchIsOver->setNextState(_displayStats.get());
    if (nbSets > 0) {
        _displayStats->setNextState(_manageMultiSets.get());
        _manageMultiSets->setNextSetState(_setupMatch.get());
        _manageMultiSets->setEndOfGameState(_podium.get());
        _podium->setNextState(_leaveGame.get());
        _setupMatch->setHandicapOnVictorious(false);
        _setupMatch->setDisplayVictories(true);
        _podium->setStoryScreenValuesProvider(_manageMultiSets.get());
    }
    else {
        _displayStats->setNextState(_setupMatch.get());
    }
    // Initializing the state machine
    _stateMachine.setInitialState(_pushGameScreen.get());
}

void AltTwoPlayersStarterAction::action(Widget *sender, int actionType,
                                        event_manager::GameControlEvent *event)
{
    _stateMachine.reset();
    _stateMachine.evaluate();
}

