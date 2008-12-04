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

class MainScreenMenu;

class MainScreen : public PuyoScreen, public gameui::SliderContainerListener {
  public:
    MainScreen(StoryWidget *fgStory = NULL, StoryWidget *bgStory = NULL);
    ~MainScreen();
    void pushMenu(MainScreenMenu *menu, bool fullScreen = false);
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
    StoryWidget *fgStory, *bgStory;
    Stack<Widget*> menuStack;
    Stack<bool> fullScreenStack;
    PuyoScreenTransitionWidget *transition;
    bool nextFullScreen;
};

class MainScreenMenu : public gameui::Frame {
public:
    MainScreenMenu(MainScreen *mainScreen, GameLoop *loop = NULL);
    virtual void build() = 0;
    // Notifications
    virtual void onMainScreenMenuPushed() {}
    virtual void onMainScreenMenuPoped() {}
protected:
    MainScreen *mainScreen;
};

class PuyoPushMenuAction : public Action
{
public:
    PuyoPushMenuAction(MainScreenMenu *menu, MainScreen *mainScreen, bool fullScreen = false)
      : mainScreen(mainScreen), menu(menu), m_fullScreen(fullScreen) {}
    void action();
private:
    MainScreen *mainScreen;
    MainScreenMenu *menu;
    bool m_fullScreen;
};

class PuyoPopMenuAction : public Action
{
public:
    PuyoPopMenuAction(MainScreen *mainScreen) : mainScreen(mainScreen) {}
    void action();
private:
    MainScreen *mainScreen;
};

#endif // _MAIN_SCREEN_H_
