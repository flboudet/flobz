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

#ifndef _PUYOMENU
#define _PUYOMENU

#include "PuyoCommander.h"
#include "sofont.h"
#include "menu.h"
#include "ios_memory.h"

class PuyoMenuScreen;

class PuyoWidget
{
public:
    PuyoWidget(PuyoMenuScreen *parentScreen, SDL_Surface *display) : parentScreen(parentScreen), display(display) {}
    virtual ~PuyoWidget() {}
    virtual void render() = 0;
    virtual void update() = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    typedef enum EventState { DONE, UNHANDLED, NEXTWIDGET, PREVIOUSWIDGET } eventState;
    virtual EventState onEvent(GameControlEvent &controlEvent, SDL_Event &e) = 0;
protected:
    PuyoMenuScreen *parentScreen;
    SDL_Surface *display;
};

class PuyoMenuScreen
{
public:
    PuyoMenuScreen(PuyoCommander &commander, SDL_Surface *display);
    virtual ~PuyoMenuScreen();
    virtual void render();
    virtual void update();
    virtual void run();
    virtual void show();
    virtual void hide();
    void addWidget(PuyoWidget *widget);
protected:
    ios_fc::AdvancedBuffer<PuyoWidget *> widgets;
    PuyoCommander &attachedCommander;
    SDL_Surface *display;
};

class PuyoMenuItem {
public:
    const char *name;
    char *value;
    int inactive;
    enum PuyoMenuItemType { NONE, EDIT } type;
};

class PuyoMenu : public PuyoWidget
{
public:
    PuyoMenu(PuyoMenuScreen *parentScreen, SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display);
    virtual ~PuyoMenu();
    virtual void render();
    virtual void update();
    virtual void show();
    virtual void hide();
    virtual EventState onEvent(GameControlEvent &controlEvent, SDL_Event &e);
    virtual void itemSelected(int itemNumber) {}
    const char *getValue(int itemNumber) const;
protected:
    void addItem(const char *itemString, const char *itemValue = NULL, bool itemInactive = false, PuyoMenuItem::PuyoMenuItemType type = PuyoMenuItem::NONE);
    void addSeparator();
    PuyoCommander &attachedCommander;
private:
    ios_fc::AdvancedBuffer<PuyoMenuItem *> itemNames;
    MenuItems items;
    Menu *menu;
    SoFont * font;
    IIM_Surface * menuselector;
    enum { NORMAL, EDITION } menuMode;
    // Edition variables
    const char *editedItemName;
    int len, maxlen;
    char prevValue[256];
    //char newValue[256];
    char out[256];
};

#endif // _PUYOMENU
