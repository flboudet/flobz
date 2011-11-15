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

#ifndef _PUYOLOCALMENU
#define _PUYOLOCALMENU

#include "gameui.h"
#include "FramedEditField.h"
#include "MainScreen.h"
#include "PuyoCommander.h"
#include "SoloGameStarter.h"
#include "PuyoSinglePlayerStarter.h"
#include "PuyoTwoPlayerStarter.h"

class SoloGameMenu : public MainScreenMenu, public PlayerNameProvider {
public:
    SoloGameMenu(MainScreen *mainScreen);
    // PlayerNameProvider implementation
    virtual String getPlayerName(int playerNumber) const;
    void build() {}
private:
    VBox m_buttonsBox;
    std::auto_ptr<EditFieldWithLabel> m_editPlayerName;
    std::auto_ptr<Frame> m_screenTitleFrame;
    std::auto_ptr<Text> m_screenTitle;
    std::auto_ptr<Button> m_easy, m_medium, m_hard, m_back;
    std::auto_ptr<SoloModeStarterAction> m_easyAction;
    std::auto_ptr<SoloModeStarterAction> m_mediumAction;
    std::auto_ptr<SoloModeStarterAction> m_hardAction;
    std::auto_ptr<PopMainScreenMenuAction> m_popAction;
};

class LocalGameMenu : public MainScreenMenu, public PlayerNameProvider {
public:
    LocalGameMenu(MainScreen *mainScreen);
    void build();
    // PlayerNameProvider implementation
    virtual String getPlayerName(int playerNumber) const;
private:
    VBox buttonsBox;
    LocalizedDictionary locale;
    EditFieldWithLabel editPlayerName;
    Frame screenTitleFrame;
    Text screenTitle;
    StoryModeStarterAction easyAction;
    StoryModeStarterAction mediumAction;
    StoryModeStarterAction hardAction;
    PopMainScreenMenuAction popAction;
    Button easy, medium, hard, back;
};

class Local2PlayersGameMenu : public MainScreenMenu, public PlayerNameProvider {
public:
    Local2PlayersGameMenu(MainScreen *mainScreen);
    void build();
    String getPlayerName(int playerNumber) const;
private:
    VBox buttonsBox;
    LocalizedDictionary locale;
    PuyoLocalTwoPlayerGameWidgetFactory gameWidgetFactory;
    EditFieldWithLabel editPlayer1Name, editPlayer2Name;
    Frame screenTitleFrame;
    Text screenTitle;
    AltTwoPlayersStarterAction easyAction;
    AltTwoPlayersStarterAction mediumAction;
    AltTwoPlayersStarterAction hardAction;
    PopMainScreenMenuAction popAction;
    Button easy, medium, hard, back;
};

#endif // _PUYOLOCALMENU
