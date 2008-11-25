/*
 *  MainScreen.h
 *  flobopuyo
 *
 *  Created by Florent Boudet on 18/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MAIN_SCREEN_H_
#define _MAIN_SCREEN_H_

#include "gameui.h"
#include "Frame.h"
#include "PuyoStory.h"

class PuyoScreen : public gameui::Screen {
  public:
    PuyoScreen();
    virtual ~PuyoScreen() {}
    virtual void build() = 0;
};

class PuyoMainScreenMenu;

class PuyoMainScreen : public PuyoScreen, public gameui::SliderContainerListener {
  public:
    PuyoMainScreen(PuyoStoryWidget *fgStory = NULL, PuyoStoryWidget *bgStory = NULL);
    ~PuyoMainScreen();
    void pushMenu(PuyoMainScreenMenu *menu, bool fullScreen = false);
    void popMenu();
    void build() {}
    void transitionFromScreen(Screen &fromScreen);
    void onEvent(GameControlEvent *cevent);
    // SliderContainerListener implementation
    /**
     * Notify that the slider is outside of the screen, before sliding back inside
     */
    virtual void onSlideOutside(SliderContainer &slider);
  protected:
    void setMenuDimensions();
    SliderContainer container;
    PuyoStoryWidget *fgStory, *bgStory;
    Stack<Widget*> menuStack;
    Stack<bool> fullScreenStack;
    PuyoScreenTransitionWidget *transition;
    bool nextFullScreen;
};

class PuyoMainScreenMenu : public gameui::Frame {
public:
    PuyoMainScreenMenu(PuyoMainScreen *mainScreen, GameLoop *loop = NULL);
    virtual void build() = 0;
    // Notifications
    virtual void onMainScreenMenuPushed() {}
    virtual void onMainScreenMenuPoped() {}
protected:
    PuyoMainScreen *mainScreen;
};

class PuyoPushMenuAction : public Action
{
public:
    PuyoPushMenuAction(PuyoMainScreenMenu *menu, PuyoMainScreen *mainScreen, bool fullScreen = false)
      : mainScreen(mainScreen), menu(menu), m_fullScreen(fullScreen) {}
    void action();
private:
    PuyoMainScreen *mainScreen;
    PuyoMainScreenMenu *menu;
    bool m_fullScreen;
};

class PuyoPopMenuAction : public Action
{
public:
    PuyoPopMenuAction(PuyoMainScreen *mainScreen) : mainScreen(mainScreen) {}
    void action();
private:
    PuyoMainScreen *mainScreen;
};

#endif // _MAIN_SCREEN_H_
