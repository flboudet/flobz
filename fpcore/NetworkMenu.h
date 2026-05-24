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
    Frame _titleFrame;
    Text _lanTitle;
    VBox _buttonsBox;
    EditFieldWithLabel _playerNameLabel, _portNumLabel;
    PopMainScreenMenuAction _cancelAction;
    Button _startButton, _cancelButton;
};

class NetworkInternetAction : public Action {
public:
    NetworkInternetAction(MainScreen * mainScreen, InternetGameMenu **menuToCreate) : _mainScreen(mainScreen), _menuToCreate(menuToCreate) {}
    void action();
private:
    MainScreen * _mainScreen;
    InternetGameMenu ** _menuToCreate;
};

class NetworkGameMenu : public MainScreenMenu {
public:
    NetworkGameMenu(MainScreen * mainScreen);
    void build();
private:
    LocalizedDictionary _locale;
    LANGameMenu  _lanGameMenu;
    InternetGameMenu *_internetGameMenu;
    NetworkInternetAction _internetAction;
    PushMainScreenMenuAction _lanAction;
    PopMainScreenMenuAction _mainScreenPopAction;
    Frame _titleFrame;
    Text _networkTitleText;
    VBox _buttonsBox;
    Button _lanGameButton, _internetGameButton, _cancelButton;
};

#endif // _PUYONETWORKMENU

