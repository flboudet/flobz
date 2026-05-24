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
    TwoPlayersGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, const std::string & aiFace, Action *gameOverAction = NULL);
    bool didPlayerWon() const { return isGameARunning(); }
    void cycle();
    StoryWidget *getOpponent();
private:
    FloboSetTheme &_attachedFloboThemeSet;
    RandomSystem _attachedRandom;
    LocalGameFactory _attachedGameFactory;
    GameView _areaA, _areaB;
    StoryWidget _opponentFace;
};

class LocalTwoPlayersGameWidgetFactory : public GameWidgetFactory {
public:
    GameWidget *createGameWidget(FloboSetTheme &floboSetTheme, LevelTheme &levelTheme, const std::string & centerFace, Action *gameOverAction)
    {
        return new TwoPlayersGameWidget(floboSetTheme, levelTheme, centerFace, gameOverAction);
    }
};

class AltTwoPlayersStarterAction : public Action {
public:
    AltTwoPlayersStarterAction(GameDifficulty difficulty, GameWidgetFactory *gameWidgetFactory,
                               PlayerNameProvider *nameProvider = NULL, int nbSets = 0);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			event_manager::GameControlEvent *event);
private:
    GameStateMachine _stateMachine;
    SharedMatchAssets _sharedAssets;
    SharedGetReadyAssets        _sharedGetReadyAssets;
    std::unique_ptr<PushScreenState>         _pushGameScreen;
    std::unique_ptr<SetupMatchState>         _setupMatch;
    std::unique_ptr<EnterPlayerReadyState>   _enterPlayersReady;
    std::unique_ptr<ExitPlayerReadyState>    _exitPlayersReady;
    std::unique_ptr<MatchPlayingState>       _matchPlaying;
    std::unique_ptr<MatchIsOverState>        _matchIsOver;
    std::unique_ptr<DisplayStatsState>       _displayStats;
    std::unique_ptr<ManageMultiSetsState>    _manageMultiSets;
    std::unique_ptr<DisplayStoryScreenState> _podium;
    std::unique_ptr<LeaveGameState>          _leaveGame;
};

#endif // _TWOPLAYERSGAMESTARTER_H_
