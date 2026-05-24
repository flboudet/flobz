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

#include "GameScreen.h"
#include "GTLog.h"

using namespace ios_fc;
using namespace event_manager;

GameScreen::GameScreen(GameWidget &gameWidget)
    : Screen(), _paused(false),
      _pauseMenu(this),
      _gameWidget(gameWidget),
      _overlayStory(NULL)
{
#ifdef DEBUG_GAMELOOP
  printf("GameScreen %x created\n", this);
  printf("   Idle: %x, Drawable:%x\n",
	 dynamic_cast<IdleComponent *>(this),
	 dynamic_cast<DrawableComponent *>(this));
#endif
    add(&_gameWidget);
    if (_gameWidget.getOpponent() != NULL) {
        _gameWidget.getOpponent()->setRenderEnabled(false);
        add(_gameWidget.getOpponent());
    }
    activeFX = &_gameWidget.getVisualFX();
    for (unsigned int i=0; i<activeFX->size(); ++i) {
        add((*activeFX)[i]);
        (*activeFX)[i]->setGameScreen(this);
    }
    _gameWidget.setAssociatedScreen(this);
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
        // case kGameMouseUp:
            pressedFromGameWidget = startPressed();
            break;
        case kBack:
            pressedFromGameWidget = backPressed();
            break;
        default:
            break;
        }
    }
	if (cevent->cursorEvent == kGameMouseUp)
		pressedFromGameWidget = startPressed();

    if (!pressedFromGameWidget)
        Screen::onEvent(cevent);
}

void GameScreen::onTransitionFromScreen(Screen &fromScreen)
{
    _transitionWidget.reset(theCommander->createScreenTransition(fromScreen));
    add(_transitionWidget.get());
}

bool GameScreen::startPressed()
{
    return _gameWidget.startPressed();
}

bool GameScreen::backPressed()
{
    if (_gameWidget.backPressed())
        return true;
    if (!_paused) {
        // Seems complicated.
        // The pause method is called from the game widget
        // because this method is virtual and can be overloaded
        // (see NetworkGameWidget)
        _gameWidget.setScreenToPaused(true);
    }
    else {
      // Same as for pause
      _pauseMenu.backPressed();
    }
    return false;
}

void GameScreen::setSuspended(bool suspended)
{
    if (_paused)
        return;
    if (suspended)
        _gameWidget.pause(false);
    else
        _gameWidget.resume();
}

void GameScreen::setPaused(bool fromControls)
{
    GTLogTrace("GameScreen::setPaused()");
    if (!_paused) {
        std::vector<VisualFX*> fx = _gameWidget.getVisualFX();
        for (unsigned int i=0; i<fx.size(); ++i)
            fx[i]->hide();
        this->add(&_pauseMenu);
        _pauseMenu.giveFocus();
        grabEventsOnWidget(&_pauseMenu);
        _paused = true;
        _gameWidget.pause();
        theCommander->setCursorVisible(true);
    }
}

void GameScreen::setResumed(bool fromControls)
{
    GTLogTrace("GameScreen::setResumed()");
    if (_paused) {
        _paused = false;
        if (_gameWidget.getOpponent() != NULL)
            _gameWidget.getOpponent()->show();
        std::vector<VisualFX*> fx = _gameWidget.getVisualFX();
        for (unsigned int i=0; i<fx.size(); ++i)
            fx[i]->show();
        ungrabEventsOnWidget(&_pauseMenu);
        this->remove(&_pauseMenu);
        this->focus(&_gameWidget);
        _gameWidget.resume();
        theCommander->setCursorVisible(false);
        _gameWidget.setScreenToResumed(fromControls);
    }
}

void GameScreen::abort()
{
    _gameWidget.abort();
}

void GameScreen::setOverlayStory(StoryWidget *story)
{
  _overlayStory = story;
  this->add(story);
}

void GameScreen::onScreenVisibleChanged(bool visible)
{
    theCommander->setCursorVisible(!visible);
    Screen::onScreenVisibleChanged(visible);
}

void GameScreen::action(Widget *sender, int actionType, GameControlEvent *event)
{
  if (sender == &_pauseMenu) {
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

