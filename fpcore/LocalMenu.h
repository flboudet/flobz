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

#ifndef _PUYOLOCALMENU
#define _PUYOLOCALMENU

#include "gameui.h"
#include "FramedEditField.h"
#include "MainScreen.h"
#include "FPCommander.h"
#include "SoloGameStarter.h"
#include "StoryModeStarter.h"
#include "TwoPlayersGameStarter.h"

class SoloGameMenu : public MainScreenMenu, public PlayerNameProvider {
public:
    SoloGameMenu(MainScreen *mainScreen);
    // PlayerNameProvider implementation
    virtual std::string getPlayerName(int playerNumber) const;
    void build() {}
private:
    VBox _buttonsBox;
    std::unique_ptr<EditFieldWithLabel> _editPlayerName;
    std::unique_ptr<Frame> _screenTitleFrame;
    std::unique_ptr<Text> _screenTitle;
    std::unique_ptr<Button> _easy, _medium, _hard, _back;
    std::unique_ptr<SoloModeStarterAction> _easyAction;
    std::unique_ptr<SoloModeStarterAction> _mediumAction;
    std::unique_ptr<SoloModeStarterAction> _hardAction;
    std::unique_ptr<PopMainScreenMenuAction> _popAction;
};

class LocalGameMenu : public MainScreenMenu, public PlayerNameProvider {
public:
    LocalGameMenu(MainScreen *mainScreen);
    void build();
    // PlayerNameProvider implementation
    virtual std::string getPlayerName(int playerNumber) const;
private:
    VBox _buttonsBox;
    LocalizedDictionary _locale;
    EditFieldWithLabel _editPlayerName;
    Frame _screenTitleFrame;
    Text _screenTitle;
    StoryModeStarterAction _easyAction;
    StoryModeStarterAction _mediumAction;
    StoryModeStarterAction _hardAction;
    PopMainScreenMenuAction _popAction;
    Button _easy, _medium, _hard, _back;
};

class Local2PlayersGameMenu : public MainScreenMenu, public PlayerNameProvider {
public:
    Local2PlayersGameMenu(MainScreen *mainScreen);
    void build();
    std::string getPlayerName(int playerNumber) const;
private:
    VBox _buttonsBox;
    LocalizedDictionary _locale;
    LocalTwoPlayersGameWidgetFactory _gameWidgetFactory;
    EditFieldWithLabel _editPlayer1Name, _editPlayer2Name;
    Frame _screenTitleFrame;
    Text _screenTitle;
    AltTwoPlayersStarterAction _easyAction;
    AltTwoPlayersStarterAction _mediumAction;
    AltTwoPlayersStarterAction _hardAction;
    PopMainScreenMenuAction _popAction;
    Button _easy, _medium, _hard, _back;
};

#endif // _PUYOLOCALMENU
