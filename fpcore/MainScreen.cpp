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
#include "PuyoCommander.h"

using namespace gameui;
using namespace event_manager;

#define WIDTH  640
#define HEIGHT 480
#define MENU_X 235
#define MENU_Y 225

MainScreen::MainScreen(StoryWidget *fgStory, StoryWidget *bgStory)
    : Screen(0,0,WIDTH,HEIGHT), fgStory(fgStory), bgStory(bgStory),
      transition(NULL), nextFullScreen(false)
{
    if (bgStory != NULL)
        add(bgStory);
    add(&container);
    if (fgStory != NULL) {
        add(fgStory);
	}
    setMenuDimensions();
    container.addListener(*this);
    container.setWhipSound(theCommander->getWhipSound());
    container.setWhopSound(theCommander->getWhopSound());
}

MainScreen::~MainScreen()
{
    if (transition != NULL) {
        delete(transition);
    }
}

void MainScreen::pushMenu(MainScreenMenu *menu, bool fullScreen)
{
    menuStack.push(container.getContentWidget());
    fullScreenStack.push(fullScreen);
    nextFullScreen = fullScreen;
    container.transitionToContent(menu);
    if (fgStory != NULL)
        fgStory->setIntegerValue("@inNetGameCenter", fullScreen ? 1 : 0);
}

void MainScreen::popMenu()
{
    if (menuStack.size() == 1)
        return;
    fullScreenStack.pop();
    nextFullScreen = fullScreenStack.top();
    container.transitionToContent(menuStack.top());
    menuStack.pop();
}

void MainScreen::onTransitionFromScreen(Screen &fromScreen)
{
    if (transition != NULL) {
        remove(transition);
        delete(transition);
    }
    transition = theCommander->createScreenTransition(fromScreen);
    add(transition);
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
    if (nextFullScreen) {
        menuPos.y = 0;
        menuPos.x = 0;
        container.setPosition(menuPos);
        container.setSize(Vec3(WIDTH, HEIGHT, 0));
        container.setBackgroundVisible(false);
        if (fgStory != NULL)
            fgStory->setIntegerValue("@inNetGameCenter", 1);
    }
    else {
        menuPos.y = MENU_Y;
        menuPos.x = MENU_X;
        container.setPosition(menuPos);
        container.setSize(Vec3(400, 250, 0)); // TODO: mettre dimensions dans GSL
        container.setBackgroundVisible(true);
        if (fgStory != NULL)
            fgStory->setIntegerValue("@inNetGameCenter", 0);
    }
}

MainScreenMenu::MainScreenMenu(MainScreen *mainScreen, GameLoop *loop)
  : Frame(theCommander->getWindowFramePicture(), loop),
    mainScreen(mainScreen)
{
    setPolicy(USE_MAX_SIZE);
}


void PushMainScreenMenuAction::action()
{
    mainScreen->pushMenu(menu, m_fullScreen);
}

void PopMainScreenMenuAction::action()
{
    mainScreen->popMenu();
}


