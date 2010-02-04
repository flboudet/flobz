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

#include "PuyoStarter.h"
#include "PuyoView.h"
#include "PuyoNetworkView.h"
#include "PuyoNetworkGame.h"

#include "audio.h"

using namespace ios_fc;
using namespace event_manager;

GameScreen::GameScreen(GameWidget &gameWidget, Screen &previousScreen)
    : Screen(), paused(false),
      pauseMenu(this),
      gameWidget(gameWidget),
      transitionWidget(theCommander->createScreenTransition(previousScreen)),
      overlayStory(NULL)
{
#ifdef DEBUG_GAMELOOP
  printf("GameScreen %x created\n", this);
  printf("   Idle: %x, Drawable:%x\n",
	 dynamic_cast<IdleComponent *>(this),
	 dynamic_cast<DrawableComponent *>(this));
#endif
    add(&gameWidget);
    if (gameWidget.getOpponent() != NULL)
        GameUIDefaults::GAME_LOOP->addIdle(gameWidget.getOpponent());

    activeFX = &gameWidget.getPuyoFX();
    for (unsigned int i=0; i<activeFX->size(); ++i) {
        add((*activeFX)[i]);
        (*activeFX)[i]->setGameScreen(this);
    }
    add(transitionWidget.get());
    gameWidget.setAssociatedScreen(this);
}

GameScreen::~GameScreen()
{
#ifdef DEBUG_GAMELOOP
  printf("GameScreen %x deleted\n", this);
  printf("   Idle: %x, Drawable:%x\n",
	 dynamic_cast<IdleComponent *>(this),
	 dynamic_cast<DrawableComponent *>(this));
  fflush(stdout);
#endif
  activeFX = NULL;
}

void GameScreen::onEvent(GameControlEvent *cevent)
{
    bool pressedFromGameWidget = false;
    if (!cevent->isUp) {
        switch (cevent->cursorEvent) {
        case kStart:
        case kGameMouseDown:
            pressedFromGameWidget = startPressed();
            break;
        case kBack:
            pressedFromGameWidget = backPressed();
            break;
        default:
            break;
        }
    }
    if (!pressedFromGameWidget)
        Screen::onEvent(cevent);
}

bool GameScreen::startPressed()
{
    return gameWidget.startPressed();
}

bool GameScreen::backPressed()
{
    if (gameWidget.backPressed())
        return true;
    if (!paused) {
        // Seems complicated.
        // The pause method is called from the game widget
        // because this method is virtual and can be overloaded
        // (see PuyoNetworkGameWidget)
        gameWidget.setScreenToPaused(true);
    }
    else {
      // Same as for pause
      pauseMenu.backPressed();
    }
    return false;
}

void GameScreen::setPaused(bool fromControls)
{
    if (!paused) {
        if (gameWidget.getOpponent() != NULL)
            gameWidget.getOpponent()->hide();
        std::vector<PuyoFX*> fx = gameWidget.getPuyoFX();
        for (unsigned int i=0; i<fx.size(); ++i)
            fx[i]->hide();
        this->add(&pauseMenu);
	pauseMenu.giveFocus();
	grabEventsOnWidget(&pauseMenu);
        paused = true;
        gameWidget.pause();
        theCommander->setCursorVisible(true);
    }
}

void GameScreen::setResumed(bool fromControls)
{
    if (paused) {
        paused = false;
        if (gameWidget.getOpponent() != NULL)
            gameWidget.getOpponent()->show();
        std::vector<PuyoFX*> fx = gameWidget.getPuyoFX();
        for (unsigned int i=0; i<fx.size(); ++i)
            fx[i]->show();
	ungrabEventsOnWidget(&pauseMenu);
        this->remove(&pauseMenu);
        this->focus(&gameWidget);
        gameWidget.resume();
        theCommander->setCursorVisible(false);
	gameWidget.setScreenToResumed(fromControls);
    }
}

void GameScreen::abort()
{
    gameWidget.abort();
}

void GameScreen::setOverlayStory(StoryWidget *story)
{
  overlayStory = story;
  this->add(story);
}

void GameScreen::onScreenVisibleChanged(bool visible)
{
    theCommander->setCursorVisible(!visible);
}

void GameScreen::action(Widget *sender, int actionType, GameControlEvent *event)
{
  if (sender == &pauseMenu) {
    switch (actionType) {
    case PauseMenu::KPauseMenuClosing_Abort:
      abort();
      break;
    case PauseMenu::KPauseMenuClosed_Continue:
      setResumed(true);
      break;
    default:
      break;
    }
  }
}

TwoPlayerGameWidget::TwoPlayerGameWidget(AnimatedPuyoSetTheme &puyoThemeSet, PuyoLevelTheme &levelTheme, Action *gameOverAction)
: attachedPuyoThemeSet(puyoThemeSet),
    attachedRandom(5), attachedGameFactory(&attachedRandom),
    areaA(&attachedGameFactory, 0, &attachedPuyoThemeSet, &levelTheme),
    areaB(&attachedGameFactory, 1, &attachedPuyoThemeSet, &levelTheme),
    controllerA(areaA, kPlayer1Down, kPlayer1Left, kPlayer1Right,
            kPlayer1TurnLeft, kPlayer1TurnRight),
    controllerB(areaB, kPlayer2Down, kPlayer2Left, kPlayer2Right,
            kPlayer2TurnLeft, kPlayer2TurnRight)
{
    initWithGUI(areaA, areaB, controllerA, controllerB, levelTheme, gameOverAction);
}

GameOptions GameOptions::FromLevel(int level) {
    GameOptions go;
    switch(level) {
        case 0:
            go.MIN_SPEED = 4;
            go.MAX_SPEED = 20;
            break;
        case 1:
            go.MIN_SPEED = 2;
            go.MAX_SPEED = 15;
            break;
        case 2:
            go.MIN_SPEED = 1;
            go.MAX_SPEED = 8;
            break;
    }
    return go;
}
