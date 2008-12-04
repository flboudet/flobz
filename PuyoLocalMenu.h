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
#include "PuyoCommander.h"
#include "PuyoSinglePlayerStarter.h"
#include "PuyoTwoPlayerStarter.h"

typedef enum SinglePlayerLevel {
    EASY=0,
    MEDIUM=1,
    HARD=2
} SinglePlayerLevel;

class LocalGameMenu : public MainScreenMenu, public PuyoSingleNameProvider {
public:
    LocalGameMenu(MainScreen *mainScreen);
    void build();
    String getPlayerName() const;
private:
    VBox buttonsBox;
    PuyoLocalizedDictionary locale;
    EditFieldWithLabel editPlayerName;
    Frame screenTitleFrame;
    Text screenTitle;
    SinglePlayerStarterAction easyAction;
    SinglePlayerStarterAction mediumAction;
    SinglePlayerStarterAction hardAction;
    PuyoPopMenuAction popAction;
    Button easy, medium, hard, back;
};

class Local2PlayersGameMenu : public MainScreenMenu, public PuyoTwoNameProvider {
public:
    Local2PlayersGameMenu(MainScreen *mainScreen);
    void build();
    String getPlayer1Name() const;
    String getPlayer2Name() const;
private:
    VBox buttonsBox;
    PuyoLocalizedDictionary locale;
    PuyoLocalTwoPlayerGameWidgetFactory gameWidgetFactory;
    EditFieldWithLabel editPlayer1Name, editPlayer2Name;
    Frame screenTitleFrame;
    Text screenTitle;
    TwoPlayersStarterAction easyAction;
    TwoPlayersStarterAction mediumAction;
    TwoPlayersStarterAction hardAction;
    PuyoPopMenuAction popAction;
    Button easy, medium, hard, back;
};

#endif // _PUYOLOCALMENU
