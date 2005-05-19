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

class OptionMenu;

class ToggleSoundFxAction : public Action {
public:
    ToggleSoundFxAction(OptionMenu &optMenu) : optMenu(optMenu) {}
    void action();
private:
    OptionMenu &optMenu;
};

class ToggleMusicAction : public Action {
public:
    ToggleMusicAction(OptionMenu &optMenu) : optMenu(optMenu) {}
    void action();
private:
    OptionMenu &optMenu;
};

class ToggleFullScreenAction : public Action {
public:
    ToggleFullScreenAction(OptionMenu &optMenu) : optMenu(optMenu) {}
    void action();
private:
    OptionMenu &optMenu;
};

class OptionMenu : public PuyoMainScreenMenu {
public:
    OptionMenu(PuyoRealMainScreen *mainScreen);
    void build();
    void toggleSoundFx();
    void toggleMusic();
    void toggleFullScreen();
private:
    ToggleSoundFxAction toggleSoundFxAction;
    ToggleMusicAction toggleMusicAction;
    ToggleFullScreenAction toggleFullScreenAction;
    ToggleButton audioButton, musicButton, fullScreenButton;
    Button changeControlsButton;
};

#endif // _PUYOOPTIONMENU

