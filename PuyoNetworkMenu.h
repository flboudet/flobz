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

#ifndef _PUYONETWORKMENU
#define _PUYONETWORKMENU

#include "PuyoMenu.h"

// LAN Game Screen
class PuyoLanGameMenu : public PuyoMenu
{
public:
    PuyoLanGameMenu(PuyoMenuScreen *parentScreen, SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display);
    void itemSelected(int itemNumber);
};

class PuyoLanGameMenuScreen : public PuyoMenuScreen
{
public:
    PuyoLanGameMenuScreen(SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display);
};

// Internet Game Screen
class PuyoInternetGameMenu : public PuyoMenu
{
public:
    PuyoInternetGameMenu(PuyoMenuScreen *parentScreen, SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display);
    void itemSelected(int itemNumber);
};

class PuyoInternetGameMenuScreen : public PuyoMenuScreen
{
public:
    PuyoInternetGameMenuScreen(SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display);
};

// Network Game Screen
class PuyoNetworkGameMenu : public PuyoMenu
{
public:
    PuyoNetworkGameMenu(PuyoMenuScreen *parentScreen, SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display);
    void itemSelected(int itemNumber);
    virtual void render();
    virtual void update();
private:
    PuyoLanGameMenuScreen *lanGameMenuScreen;
    PuyoInternetGameMenuScreen *internetGameMenuScreen;
};

class PuyoNetworkGameMenuScreen : public PuyoMenuScreen
{
public:
    PuyoNetworkGameMenuScreen(SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display);
};

#endif // _PUYONETWORKMENU

