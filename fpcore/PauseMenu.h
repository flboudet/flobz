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

#ifndef _PUYO_PAUSE_MENU_H
#define _PUYO_PAUSE_MENU_H

#include "Frame.h"
#include "OptionMenu.h"

/**
 * The class PauseMenu implements the ingame pause menu shown
 * when the player presses the pause button (usually esc)
 */
class PauseMenu : public VBox, public Action, public SliderContainerListener {
public:
    PauseMenu(Action *pauseAction);
    virtual ~PauseMenu();
    int pauseMenuTop, pauseMenuLeft;
    void backPressed(bool fromControls = true);
    // Action handler
    virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
    /**
     * Notify that the slider is outside of the screen, before sliding back inside
     */
    virtual void onSlideInside(SliderContainer &slider);
    virtual void onWidgetAdded(WidgetContainer *parent);
    /**
     * The different action types sent by the pause menu
     */
    enum ActionTypes {
        KPauseMenuClosing_Continue,
        KPauseMenuClosing_Abort,
        KPauseMenuClosed_Continue
    };
private:
    SliderContainer pauseContainer;
    Separator topSeparator;
    HBox topBox;
    Frame pauseVBox;
    Frame pauseTitleFrame;
    Text menuTitle;
    VBox buttonsBox;
    Button continueButton, optionsButton;
    AudioPrefSwitch audioButton;
    MusicPrefSwitch musicButton;
    FSPrefSwitch fullScreenButton;
    Button abortButton;
    Frame optionsBox;
    Frame optionsTitleFrame;
    Text optionsTitle;
    VBox optionsButtonsBox;
    Button optionsBack;
    Action *m_pauseAction;
};

#endif // _PUYO_PAUSE_MENU_H

