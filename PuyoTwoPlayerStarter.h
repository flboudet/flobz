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

class PuyoTwoNameProvider {
public:
    virtual String getPlayer1Name() const = 0;
    virtual String getPlayer2Name() const = 0;
};

class PuyoTwoPlayersGameWidget : public PuyoGameWidget {
public:
    PuyoTwoPlayersGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, String aiFace, Action *gameOverAction = NULL);
    bool didPlayerWon() const { return isGameARunning(); }
    PuyoStoryWidget *getOpponent();
private:
    AnimatedPuyoSetTheme &attachedPuyoThemeSet;
    PuyoRandomSystem attachedRandom;
    PuyoLocalGameFactory attachedGameFactory;
    PuyoView areaA, areaB;
    PuyoEventPlayer playercontrollerA, playercontrollerB;
    PuyoStoryWidget opponentFace;
};


class PuyoGameWidgetFactory {
public:
    virtual PuyoGameWidget *createGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, String centerFace, Action *gameOverAction) = 0;
};

class PuyoLocalTwoPlayerGameWidgetFactory : public PuyoGameWidgetFactory {
public:
    PuyoGameWidget *createGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, String centerFace, Action *gameOverAction)
    {
        return new PuyoTwoPlayersGameWidget(puyoThemeSet, levelTheme, centerFace, gameOverAction);
    }
};

class TwoPlayersStarterAction : public Action {
public:
    TwoPlayersStarterAction(int difficulty, PuyoGameWidgetFactory &gameWidgetFactory, PuyoTwoNameProvider *nameProvider = NULL);
    void action();
    
private:    
    void startGame();
    void gameOver();
    void restartGame();
    void endGameSession();
    
    int difficulty;
    PuyoGameWidgetFactory &gameWidgetFactory;
    PuyoGameScreen *gameScreen;
    PuyoGameWidget *gameWidget;
    PuyoTwoNameProvider *nameProvider;
    PuyoStoryWidget *gameLostWidget;
    
    PuyoLevelTheme *currentLevelTheme;
    int leftVictories, rightVictories;
};

#endif // _PUYOTWOPLAYERSTARTER
