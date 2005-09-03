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

PuyoTwoPlayersGameWidget::PuyoTwoPlayersGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, Action *gameOverAction) : attachedPuyoThemeSet(puyoThemeSet),
                                                     attachedGameFactory(&attachedRandom),
                                                     areaA(&attachedGameFactory, &attachedPuyoThemeSet,
                                                     1 + CSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + FSIZE, BSIZE+ESIZE, painter),
                                                     areaB(&attachedGameFactory, &attachedPuyoThemeSet,
                                                     1 + CSIZE + PUYODIMX*TSIZE + DSIZE, BSIZE-TSIZE, CSIZE + PUYODIMX*TSIZE + DSIZE - FSIZE - TSIZE, BSIZE+ESIZE, painter),
                                                     playercontrollerA(areaA, GameControlEvent::kPlayer1Down, GameControlEvent::kPlayer1Left, GameControlEvent::kPlayer1Right,
                                                     GameControlEvent::kPlayer1TurnLeft, GameControlEvent::kPlayer1TurnRight),
                                                     playercontrollerB(areaB, GameControlEvent::kPlayer2Down, GameControlEvent::kPlayer2Left, GameControlEvent::kPlayer2Right,
                                                     GameControlEvent::kPlayer2TurnLeft, GameControlEvent::kPlayer2TurnRight)
{
    initialize(areaA, areaB, playercontrollerA, playercontrollerB, levelTheme, gameOverAction);
    setLives(-1);
}

TwoPlayersStarterAction::TwoPlayersStarterAction(int difficulty, PuyoGameWidgetFactory &gameWidgetFactory, PuyoTwoNameProvider *nameProvider)
    : difficulty(difficulty), gameWidgetFactory(gameWidgetFactory), gameScreen(NULL), nameProvider(nameProvider) {}

void TwoPlayersStarterAction::action()
{
    if (gameScreen == NULL) {
        startGame();
    }
    else if (! gameWidget->getAborted()) {
        gameOver();
    }
    else {
	    endGameSession();
	}
}

void TwoPlayersStarterAction::startGame()
{
    AnimatedPuyoThemeManager * themeManager = getPuyoThemeManger();
    
    gameWidget = gameWidgetFactory.createGameWidget(*(themeManager->getAnimatedPuyoSetTheme()), *(themeManager->getPuyoLevelTheme()), this);
    gameScreen = new PuyoGameScreen(*gameWidget, *(GameUIDefaults::SCREEN_STACK->top()));
    if (nameProvider != NULL) {
        gameWidget->setPlayerOneName(nameProvider->getPlayer1Name());
        gameWidget->setPlayerTwoName(nameProvider->getPlayer2Name());
    }
    GameUIDefaults::SCREEN_STACK->push(gameScreen);
}

void TwoPlayersStarterAction::gameOver()
{
    GameUIDefaults::SCREEN_STACK->pop();
    if (gameWidget != NULL) {
        delete gameWidget;
        delete gameScreen;
    }
    AnimatedPuyoThemeManager * themeManager = getPuyoThemeManger();
    
    PuyoTwoPlayersGameWidget *newGameWidget = new PuyoTwoPlayersGameWidget(*(themeManager->getAnimatedPuyoSetTheme()), *(themeManager->getPuyoLevelTheme()), this);
    PuyoGameScreen *newGameScreen = new PuyoGameScreen(*gameWidget, *(GameUIDefaults::SCREEN_STACK->top()));
    if (nameProvider != NULL) {
        gameWidget->setPlayerOneName(nameProvider->getPlayer1Name());
        gameWidget->setPlayerTwoName(nameProvider->getPlayer2Name());
    }
    /*
    GameUIDefaults::SCREEN_STACK->pop();
    if (gameWidget != NULL) {
        delete gameWidget;
        delete gameScreen;
    }*/
    gameScreen = newGameScreen;
    gameWidget = newGameWidget;
    GameUIDefaults::SCREEN_STACK->push(gameScreen);
}

void TwoPlayersStarterAction::endGameSession()
{
    GameUIDefaults::SCREEN_STACK->pop();
    ((PuyoRealMainScreen *)(GameUIDefaults::SCREEN_STACK->top()))->transitionFromScreen(*gameScreen);
    delete gameWidget;
    delete gameScreen;
    
    gameScreen = NULL;
    gameWidget = NULL;
}

