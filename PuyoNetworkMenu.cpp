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

#include "PuyoNetworkMenu.h"
#include "preferences.h"

// LAN Game Screen
PuyoLanGameMenu::PuyoLanGameMenu(PuyoMenuScreen *parentScreen, SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display) : PuyoMenu(parentScreen, font, menuselector, commander, display)
{
    char playerName[256];
    char ipAddress[256];

    GetStrPreference("Player Name", playerName, "Player 1");
    GetStrPreference("IP Address", ipAddress,   "");
    
    addItem("Player Name:\t", playerName, false, PuyoMenuItem::EDIT);
    addItem("IP Address:\t", ipAddress, false, PuyoMenuItem::EDIT);
    addSeparator();
    addSeparator();
    addSeparator();
    addItem("Choose Game Level", NULL, true);
    addSeparator();
    addItem("Easy");
    addItem("Medium");
    addItem("Hard");
}

void PuyoLanGameMenu::itemSelected(int itemNumber)
{
    switch (itemNumber) {
        case 7:
        case 8:
        case 9:
            parentScreen->hide();
            attachedCommander.startLANGame(itemNumber - 7, getValue(0), getValue(1));
            parentScreen->show();
            break;
        default:
            break;
    }
}

PuyoLanGameMenuScreen::PuyoLanGameMenuScreen(SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display)
:PuyoMenuScreen(commander, display)
{
    addWidget(new PuyoLanGameMenu(this, font, menuselector, commander, display));
}

// Internet Game Screen
PuyoInternetGameMenu::PuyoInternetGameMenu(PuyoMenuScreen *parentScreen, SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display) : PuyoMenu(parentScreen, font, menuselector, commander, display)
{
    char playerName[256];
    char serverAddress[256];

    GetStrPreference("Player Name", playerName, "Player 1");
    GetStrPreference("Server Address", serverAddress,   "durandal.homeunix.com");
    
    addItem("Player Name:\t", playerName, false, PuyoMenuItem::EDIT);
    addItem("Opponent Name:\t", "Joe le rat", false, PuyoMenuItem::EDIT);
    addSeparator();
    addItem("Server Address:\t", serverAddress, false, PuyoMenuItem::EDIT);
    addSeparator();
    addSeparator();
    addSeparator();
    addItem("Choose Game Level", NULL, true);
    addSeparator();
    addItem("Easy");
    addItem("Medium");
    addItem("Hard");
}

void PuyoInternetGameMenu::itemSelected(int itemNumber)
{
    // work in progress
    switch (itemNumber) {
        case 10:
            parentScreen->hide();
            attachedCommander.startInternetGame(0, "TEST", "localhost", 4567, 32776);
            parentScreen->show();
            break;
        default:
            break;
    }
}

PuyoInternetGameMenuScreen::PuyoInternetGameMenuScreen(SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display)
:PuyoMenuScreen(commander, display)
{
    addWidget(new PuyoInternetGameMenu(this, font, menuselector, commander, display));
}

// Network Game Screen
PuyoNetworkGameMenu::PuyoNetworkGameMenu(PuyoMenuScreen *parentScreen, SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display) : PuyoMenu(parentScreen, font, menuselector, commander, display)
{
    addItem("LAN Game");
    addItem("Internet Game");
    lanGameMenuScreen = new PuyoLanGameMenuScreen(font, menuselector, commander, display);
    internetGameMenuScreen = new PuyoInternetGameMenuScreen(font, menuselector, commander, display);
}

void PuyoNetworkGameMenu::itemSelected(int itemNumber)
{
    switch (itemNumber) {
        case 0:
            parentScreen->hide();
            lanGameMenuScreen->run();
            parentScreen->show();
            break;
        case 1:
            parentScreen->hide();
            internetGameMenuScreen->run();
            parentScreen->show();
            break;
        default:
            break;
    }
}

void PuyoNetworkGameMenu::render()
{
    lanGameMenuScreen->render();
    internetGameMenuScreen->render();
    PuyoMenu::render();
}

void PuyoNetworkGameMenu::update()
{
    lanGameMenuScreen->update();
    internetGameMenuScreen->update();
    PuyoMenu::update();
}

PuyoNetworkGameMenuScreen::PuyoNetworkGameMenuScreen(SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display)
:PuyoMenuScreen(commander, display)
{
    addWidget(new PuyoNetworkGameMenu(this, font, menuselector, commander, display));
}

