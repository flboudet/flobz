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

#include "MainScreen.h"
#include "FPCommander.h"

using namespace gameui;
using namespace event_manager;

#define WIDTH  640
#define HEIGHT 480
#define MENU_X 235
#define MENU_Y 225

MainScreen::MainScreen(StoryWidget *fgStory, StoryWidget *bgStory)
    : Screen(0,0,WIDTH,HEIGHT), _fgStory(fgStory), _bgStory(bgStory),
      _transition(NULL), _nextFullScreen(false)
{
    if (_bgStory != NULL)
        add(_bgStory);
    add(&_container);
    if (_fgStory != NULL) {
        add(_fgStory);
	}
    setMenuDimensions();
    _container.addListener(*this);
    _container.setWhipSound(theCommander->getWhipSound());
    _container.setWhopSound(theCommander->getWhopSound());
}

MainScreen::~MainScreen()
{
    if (_transition != NULL) {
        delete(_transition);
    }
}

void MainScreen::pushMenu(MainScreenMenu *menu, bool fullScreen)
{
    _menuStack.push_back(_container.getContentWidget());
    _fullScreenStack.push_back(fullScreen);
    _nextFullScreen = fullScreen;
    _container.transitionToContent(menu);
    if (_fgStory != NULL)
        _fgStory->setIntegerValue("@inNetGameCenter", fullScreen ? 1 : 0);
}

void MainScreen::popMenu()
{
    if (_menuStack.size() == 1)
        return;
    _fullScreenStack.pop_back();
    _nextFullScreen = _fullScreenStack.back();
    _container.transitionToContent(_menuStack.back());
    _menuStack.pop_back();
}

void MainScreen::onTransitionFromScreen(Screen &fromScreen)
{
    if (_transition != NULL) {
        remove(_transition);
        delete(_transition);
    }
    _transition = theCommander->createScreenTransition(fromScreen);
    add(_transition);
    setMenuDimensions();
}

void MainScreen::onEvent(GameControlEvent *cevent)
{
    gameui::Screen::onEvent(cevent);
	if (cevent->caught != false)
		return;
    if (cevent->isUp)
        return;
    switch (cevent->cursorEvent) {
    case kStart:
        break;
    case kBack:
        popMenu();
        break;
      default:
        break;
    }
}

void MainScreen::onSlideOutside(SliderContainer &slider)
{
    setMenuDimensions();
}

void MainScreen::setMenuDimensions()
{
	Vec3 menuPos;
    if (_nextFullScreen) {
        menuPos.y = 0;
        menuPos.x = 0;
        _container.setPosition(menuPos);
        _container.setSize(Vec3(WIDTH, HEIGHT, 0));
        _container.setBackgroundVisible(false);
        if (_fgStory != NULL)
            _fgStory->setIntegerValue("@inNetGameCenter", 1);
    }
    else {
        menuPos.y = MENU_Y;
        menuPos.x = MENU_X;
        _container.setPosition(menuPos);
        _container.setSize(Vec3(400, 250, 0)); // TODO: mettre dimensions dans GSL
        _container.setBackgroundVisible(true);
        if (_fgStory != NULL)
            _fgStory->setIntegerValue("@inNetGameCenter", 0);
    }
}

MainScreenMenu::MainScreenMenu(MainScreen *mainScreen, GameLoop *loop)
  : Frame(theCommander->getWindowFramePicture(), loop),
    _mainScreen(mainScreen)
{
    setPolicy(USE_MAX_SIZE);
}


void PushMainScreenMenuAction::action()
{
    _mainScreen->pushMenu(_menu, _fullScreen);
}

void PopMainScreenMenuAction::action()
{
    _mainScreen->popMenu();
}


