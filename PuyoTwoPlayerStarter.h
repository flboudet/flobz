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
    TwoPlayersGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, String aiFace, Action *gameOverAction = NULL);
    bool didPlayerWon() const { return isGameARunning(); }
    void cycle();
    StoryWidget *getOpponent();
private:
    AnimatedPuyoSetTheme &attachedPuyoThemeSet;
    PuyoRandomSystem attachedRandom;
    PuyoLocalGameFactory attachedGameFactory;
    PuyoView areaA, areaB;
    PuyoEventPlayer playercontrollerA, playercontrollerB;
    StoryWidget opponentFace;
};


class GameWidgetFactory {
public:
    virtual GameWidget *createGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, String centerFace, Action *gameOverAction) = 0;
    virtual ~GameWidgetFactory() {};
};

class PuyoLocalTwoPlayerGameWidgetFactory : public GameWidgetFactory {
public:
    GameWidget *createGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, String centerFace, Action *gameOverAction)
    {
        return new TwoPlayersGameWidget(puyoThemeSet, levelTheme, centerFace, gameOverAction);
    }
};

class TwoPlayersStarterAction : public Action {
public:
    TwoPlayersStarterAction(int difficulty, GameWidgetFactory &gameWidgetFactory, PuyoTwoNameProvider *nameProvider = NULL);
    /**
     * Implements the Action interface
     */
    virtual void action(Widget *sender, int actionType,
			GameControlEvent *event);
private:
    enum State {
      kNotRunning,
      kMatchPlaying,
      kMatchWonP1Animation,
      kMatchWonP2Animation,
      kMatchScores,
    };
    /**
     * Performs a step in the match state machine
     */
    void stateMachine();
    void startGame();
    void gameOver();
    void gameScores();
    void restartGame();
    void endGameSession();
    State m_state;
    int difficulty;
    GameWidgetFactory &gameWidgetFactory;
    GameScreen *gameScreen;
    GameWidget *gameWidget;
    PuyoTwoNameProvider *nameProvider;
    StoryWidget *gameLostWidget;

    PuyoLevelTheme *currentLevelTheme;
    int leftVictories, rightVictories;
    TwoPlayersStatsWidget *m_statsWidget;
    int totalPointsPlayerOne_, totalPointsPlayerTwo_;
};

#endif // _PUYOTWOPLAYERSTARTER
