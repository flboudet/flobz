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

#define WIDTH  640
#define HEIGHT 480
#define MENU_X 235
#define MENU_Y 225

PuyoScreen::PuyoScreen() : Screen(0,0,WIDTH,HEIGHT) {}

PuyoMainScreen::PuyoMainScreen(PuyoStoryWidget *fgStory, PuyoStoryWidget *bgStory)
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
}

PuyoMainScreen::~PuyoMainScreen()
{
    if (transition != NULL) {
        delete(transition);
    }
}

void PuyoMainScreen::pushMenu(PuyoMainScreenMenu *menu, bool fullScreen)
{
    menuStack.push(container.getContentWidget());
    fullScreenStack.push(fullScreen);
    nextFullScreen = fullScreen;
    container.transitionToContent(menu);
    if (fgStory != NULL)
        fgStory->setIntegerValue("@inNetGameCenter", fullScreen ? 1 : 0);
}

void PuyoMainScreen::popMenu()
{
    if (menuStack.size() == 1)
        return;
    fullScreenStack.pop();
    nextFullScreen = fullScreenStack.top();
    container.transitionToContent(menuStack.top());
    menuStack.pop();
}

void PuyoMainScreen::transitionFromScreen(Screen &fromScreen)
{
    if (transition != NULL) {
        remove(transition);
        delete(transition);
    }
    transition = new PuyoScreenTransitionWidget(fromScreen, NULL);
    add(transition);
    setMenuDimensions();
}

void PuyoMainScreen::onEvent(GameControlEvent *cevent)
{
    PuyoScreen::onEvent(cevent);
	if (cevent->caught != false)
		return;
    if (cevent->isUp)
        return;
    switch (cevent->cursorEvent) {
    case GameControlEvent::kStart:
        break;
    case GameControlEvent::kBack:
        popMenu();
        break;
      default:
        break;
    }
}

void PuyoMainScreen::onSlideOutside(SliderContainer &slider)
{
    setMenuDimensions();
}

void PuyoMainScreen::setMenuDimensions()
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

PuyoMainScreenMenu::PuyoMainScreenMenu(PuyoMainScreen *mainScreen, GameLoop *loop)
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

