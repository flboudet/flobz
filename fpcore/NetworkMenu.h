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

#ifndef _PUYONETWORKMENU
#define _PUYONETWORKMENU

#include "gameui.h"
#include "Frame.h"
#include "FramedButton.h"
#include "FramedEditField.h"
#include "ListView.h"
#include "FPCommander.h"
#include "MainScreen.h"
#include "InternetMenu.h"
#include "InternetGameCenter.h"


class LANGameMenu : public MainScreenMenu {
public:
    LANGameMenu(MainScreen * mainScreen);
    void build();
private:
    Frame titleFrame;
    Text lanTitle;
    VBox buttonsBox;
    EditFieldWithLabel playerNameLabel, portNumLabel;
    PopMainScreenMenuAction cancelAction;
    Button startButton, cancelButton;
};

class NetworkInternetAction : public Action {
public:
    NetworkInternetAction(MainScreen * mainScreen, InternetGameMenu **menuToCreate) : mainScreen(mainScreen), menuToCreate(menuToCreate) {}
    void action();
private:
    MainScreen * mainScreen;
    InternetGameMenu **menuToCreate;
};

class NetworkGameMenu : public MainScreenMenu {
public:
    NetworkGameMenu(MainScreen * mainScreen);
    void build();
private:
    LocalizedDictionary locale;
    LANGameMenu  lanGameMenu;
    InternetGameMenu *internetGameMenu;
    NetworkInternetAction internetAction;
    PushMainScreenMenuAction lanAction;
    PopMainScreenMenuAction mainScreenPopAction;
    Frame titleFrame;
    Text networkTitleText;
    VBox buttonsBox;
    Button lanGameButton, internetGameButton, cancelButton;
};

#endif // _PUYONETWORKMENU

