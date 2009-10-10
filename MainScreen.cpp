/*
 *  MainScreen.cpp
 *  flobopuyo
 *
 *  Created by Florent Boudet on 18/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
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

PuyoScreen::PuyoScreen() : Screen(0,0,WIDTH,HEIGHT) {}

MainScreen::MainScreen(StoryWidget *fgStory, StoryWidget *bgStory)
    : fgStory(fgStory), bgStory(bgStory), transition(NULL), nextFullScreen(false)
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

void MainScreen::transitionFromScreen(Screen &fromScreen)
{
    if (transition != NULL) {
        remove(transition);
        delete(transition);
    }
    transition = new ScreenTransitionWidget(fromScreen, NULL);
    add(transition);
    setMenuDimensions();
}

void MainScreen::onEvent(GameControlEvent *cevent)
{
    PuyoScreen::onEvent(cevent);
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


void PuyoPushMenuAction::action()
{
    mainScreen->pushMenu(menu, m_fullScreen);
}

void PuyoPopMenuAction::action()
{
    mainScreen->popMenu();
}


