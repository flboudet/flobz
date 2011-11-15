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

#ifndef _PUYOOPTIONMENU
#define _PUYOOPTIONMENU

#include "gameui.h"
#include "FPCommander.h"
#include "ThemeMenu.h"
#include "LevelThemeMenu.h"
#include "ControlMenu.h"
#include "SwitchedButton.h"

class OptionMenu;

class MusicPrefSwitch : public gameui::SwitchedButton {
public:
    MusicPrefSwitch();
};

class AudioPrefSwitch : public gameui::SwitchedButton {
public:
    AudioPrefSwitch();
};

class FSPrefSwitch : public gameui::SwitchedButton {
public:
    FSPrefSwitch();
};

class OptionMenu : public MainScreenMenu {
public:
    OptionMenu(MainScreen *mainScreen);
    void build();
private:
    Frame screenTitleFrame;
    Text optionTitle;
    VBox buttonsBox;
    AudioPrefSwitch audioButton;
    MusicPrefSwitch musicButton;
    FSPrefSwitch fullScreenButton;
    ControlMenu controlMenu;
    ThemeMenu themeMenu;
    LevelThemeMenu levelMenu;
    PushMainScreenMenuAction changeThemeAction, changeLevelAction, changeControlsAction;
    Button changeFloboThemeButton;
    Button changeLevelThemeButton;
    Button changeControlsButton;
    PopMainScreenMenuAction backAction;
    Button backButton;
};

#endif // _PUYOOPTIONMENU

