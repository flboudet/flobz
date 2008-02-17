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

#ifndef _PUYOOPTIONMENU
#define _PUYOOPTIONMENU

#include "gameui.h"
#include "PuyoCommander.h"
#include "PuyoThemeMenu.h"
#include "LevelThemeMenu.h"
#include "PuyoControlMenu.h"

class OptionMenu;

class ToggleSoundFxAction : public Action {
public:
    ToggleSoundFxAction() : toggleButton(NULL) {}
    void setButton(ToggleButton *_toggleButton) { toggleButton = _toggleButton; }
    void action();
private:
    ToggleButton *toggleButton;
};

class ToggleMusicAction : public Action {
public:
    ToggleMusicAction() : toggleButton(NULL) {}
    void setButton(ToggleButton *_toggleButton) { toggleButton = _toggleButton; }
    void action();
private:
    ToggleButton *toggleButton;
};

class ToggleFullScreenAction : public Action {
public:
    ToggleFullScreenAction() : toggleButton(NULL) {}
    void setButton(ToggleButton *_toggleButton) { toggleButton = _toggleButton; }
    void action();
private:
    ToggleButton *toggleButton;
};

class OptionMenu : public PuyoMainScreenMenu {
public:
    OptionMenu(PuyoMainScreen *mainScreen);
    void build();
private:
    ToggleSoundFxAction toggleSoundFxAction;
    ToggleMusicAction toggleMusicAction;
    ToggleFullScreenAction toggleFullScreenAction;
    Frame screenTitleFrame;
    Text optionTitle;
    VBox buttonsBox;
    ToggleButton audioButton, musicButton, fullScreenButton;
    ControlMenu controlMenu;
    PuyoThemeMenu themeMenu;
    LevelThemeMenu levelMenu;
    PuyoPushMenuAction changeThemeAction, changeLevelAction, changeControlsAction;
    Button changePuyoThemeButton;
    Button changeLevelThemeButton;
    Button changeControlsButton;
    PuyoPopMenuAction backAction;
    Button backButton;
};

#endif // _PUYOOPTIONMENU

