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

#ifndef _FLOBO_PAUSE_MENU_H
#define _FLOBO_PAUSE_MENU_H

#include "gameui.h"
#include "Frame.h"
//#include "OptionMenu.h"

/**
 * The class PauseMenu implements the ingame pause menu shown
 * when the player presses the pause button (usually esc)
 */
class PauseMenu : public gameui::VBox,
                  public gameui::Action, public gameui::SliderContainerListener {
public:
    PauseMenu(gameui::Action *pauseAction);
    virtual ~PauseMenu();
    int pauseMenuTop, pauseMenuLeft;
    void backPressed(bool fromControls = true);
    // Action handler
    virtual void action(gameui::Widget *sender, int actionType, event_manager::GameControlEvent *event);
    /**
     * Notify that the slider is outside of the screen, before sliding back inside
     */
    virtual void onSlideInside(gameui::SliderContainer &slider);
    virtual void onWidgetAdded(gameui::WidgetContainer *parent);
    /**
     * The different action types sent by the pause menu
     */
    enum ActionTypes {
        KPauseMenuClosing_Continue,
        KPauseMenuClosing_Abort,
        KPauseMenuClosed_Continue
    };
private:
    gameui::SliderContainer pauseContainer;
    gameui::Separator topSeparator;
    gameui::HBox topBox;
    gameui::Frame pauseVBox;
    gameui::Frame pauseTitleFrame;
    gameui::Text menuTitle;
    gameui::VBox buttonsBox;
    gameui::Button continueButton;
    //Button optionsButton;
    //AudioPrefSwitch audioButton;
    //MusicPrefSwitch musicButton;
    //FSPrefSwitch fullScreenButton;
    gameui::Button abortButton;
    gameui::Frame optionsBox;
    gameui::Frame optionsTitleFrame;
    gameui::Text optionsTitle;
    gameui::VBox optionsButtonsBox;
    gameui::Button optionsBack;
    gameui::Action *m_pauseAction;
};

#endif // _FLOBO_PAUSE_MENU_H

