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

#ifndef _MAIN_SCREEN_H_
#define _MAIN_SCREEN_H_

#include "gameui.h"
#include "Frame.h"
#include "PuyoStory.h"

class MainScreenMenu;

class MainScreen : public gameui::Screen, public gameui::SliderContainerListener {
  public:
    MainScreen(StoryWidget *fgStory = NULL, StoryWidget *bgStory = NULL);
    ~MainScreen();
    // Screen implementation
    virtual void onEvent(event_manager::GameControlEvent *cevent);
    virtual void onTransitionFromScreen(Screen &fromScreen);
    // Own methods
    void pushMenu(MainScreenMenu *menu, bool fullScreen = false);
    void popMenu();
    void build() {}
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
    ScreenTransitionWidget *transition;
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

class PushMainScreenMenuAction : public Action
{
public:
    PushMainScreenMenuAction(MainScreenMenu *menu, MainScreen *mainScreen, bool fullScreen = false)
      : mainScreen(mainScreen), menu(menu), m_fullScreen(fullScreen) {}
    void action();
private:
    MainScreen *mainScreen;
    MainScreenMenu *menu;
    bool m_fullScreen;
};

class PopMainScreenMenuAction : public Action
{
public:
    PopMainScreenMenuAction(MainScreen *mainScreen) : mainScreen(mainScreen) {}
    void action();
private:
    MainScreen *mainScreen;
};

#endif // _MAIN_SCREEN_H_
