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
#include "PuyoCommander.h"
#include "PuyoSinglePlayerStarter.h"
#include "PuyoTwoPlayerStarter.h"

typedef enum SinglePlayerLevel {
    EASY=0,
    MEDIUM=1,
    HARD=2
} SinglePlayerLevel;

class LocalGameMenu : public PuyoMainScreenMenu {
public:
    LocalGameMenu(PuyoRealMainScreen *mainScreen) : PuyoMainScreenMenu(mainScreen), easyAction(EASY), mediumAction(MEDIUM), hardAction(HARD), popAction(mainScreen) {}
    void build();
private:
    SinglePlayerStarterAction easyAction;
    SinglePlayerStarterAction mediumAction;
    SinglePlayerStarterAction hardAction;
    PuyoPopMenuAction popAction;
};

class Local2PlayersGameMenu : public PuyoMainScreenMenu {
public:
    Local2PlayersGameMenu(PuyoRealMainScreen *mainScreen) : PuyoMainScreenMenu(mainScreen), easyAction(EASY), mediumAction(MEDIUM), hardAction(HARD), popAction(mainScreen) {}
    void build();
private:
    TwoPlayersStarterAction easyAction;
    TwoPlayersStarterAction mediumAction;
    TwoPlayersStarterAction hardAction;
    PuyoPopMenuAction popAction;
};

#endif // _PUYOLOCALMENU
