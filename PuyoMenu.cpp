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
/*
#include <stdlib.h>
#include "PuyoMenu.h"
#include "InputManager.h"
#include "GameControls.h"

PuyoMenuScreen::PuyoMenuScreen(PuyoCommander &commander, SDL_Surface *display) : attachedCommander(commander), display(display)
{
}

PuyoMenuScreen::~PuyoMenuScreen()
{
    for (int i = 0, j = widgets.size() ; i < j ; i++) {
        delete widgets[i];
    }
}

void PuyoMenuScreen::render()
{
    for (int i = 0, j = widgets.size() ; i < j ; i++) {
        widgets[i]->render();
    }
}

void PuyoMenuScreen::update()
{
    for (int i = 0, j = widgets.size() ; i < j ; i++) {
        widgets[i]->update();
    }
}

void PuyoMenuScreen::run()
{
    bool exitLoop = false;
    int activeWidget = 0;
    
    show();
    while (!exitLoop) {
        SDL_Event e;
        
        while (SDL_PollEvent (&e)) {
            GameControlEvent controlEvent;
            getControlEvent(e, &controlEvent);
            
            switch (widgets[activeWidget]->onEvent(controlEvent, e)) {
                case PuyoWidget::UNHANDLED:
                    switch (controlEvent.cursorEvent) {
                        case GameControlEvent::kQuit:
                            exit(0);
                            break;
                        case GameControlEvent::kBack:
                            exitLoop = true;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
        attachedCommander.updateAll(NULL);
    }
    hide();
}

void PuyoMenuScreen::show()
{
    for (int i = 0, j = widgets.size() ; i < j ; i++) {
        widgets[i]->show();
    }
}

void PuyoMenuScreen::hide()
{
    for (int i = 0, j = widgets.size() ; i < j ; i++) {
        widgets[i]->hide();
    }
}

void PuyoMenuScreen::addWidget(PuyoWidget *widget)
{
    widgets.add(widget);
}

PuyoMenu::PuyoMenu(PuyoMenuScreen *parentScreen, SoFont * font, IIM_Surface * menuselector, PuyoCommander &commander, SDL_Surface *display) : PuyoWidget(parentScreen, display), font(font), menuselector(menuselector), attachedCommander(commander)
{
    items = NULL;
    menu = NULL;
}

PuyoMenu::~PuyoMenu()
{
    if (items != NULL)
        delete items;

    for (int i = 0 ; i < itemNames.size() ; i++) {
        delete itemNames[i];
    }
}

void PuyoMenu::render()
{
    // Building menu
    if (menu == NULL) {
        items = new MenuItem[itemNames.size() + 1];
        for (int i = 0 ; i < itemNames.size() ; i++) {
            items[i].name = itemNames[i]->name;
            items[i].value = itemNames[i]->value;
            items[i].bitmap = NULL;
            items[i].font = NULL;
            items[i].priv_vx = 0;
            items[i].priv_x = 0;
            items[i].priv_must_anim = 0;
            items[i].inactive = itemNames[i]->inactive;
        }
        // End menu item
        int endItem = itemNames.size();
        items[endItem].name = NULL;
        items[endItem].value = NULL;
        items[endItem].bitmap = NULL;
        items[endItem].font = NULL;
        items[endItem].priv_vx = 0;
        items[endItem].priv_x = 0;
        items[endItem].priv_must_anim = 0;
        items[endItem].inactive = 1;
        // Font setting
        for (int i = 0 ; i < itemNames.size() ; i++) {
            if (strcmp(items[i].name, "---") != 0)
                menu_items_set_font_for (items, itemNames[i]->name, font);
        }
        // menu creation
        menu = menu_new(items, menuselector);
        menu_set_sounds (menu,     sound_pop, sound_slide);
    }
    
    if (menu != NULL) {
        menu_draw(menu, display);
    }
}

void PuyoMenu::update()
{
    if (menu != NULL) {
        menu_update(menu, display);
    }
}

void PuyoMenu::show()
{
    menu_show(menu);
}

void PuyoMenu::hide()
{
    menu_hide(menu);
}

PuyoWidget::EventState PuyoMenu::onEvent(GameControlEvent &controlEvent, SDL_Event &e)
{
    PuyoWidget::EventState result = DONE;
    
    switch (menuMode) {
        case EDITION:
            switch (controlEvent.cursorEvent) {
                case GameControlEvent::kStart:
                    out[len] = 0;
                    menu_set_value(menu, editedItemName, out);
                    menu_validate (menu);
                    menuMode = NORMAL;
                    itemSelected(menu_get_active_item(menu));
                    break;
                case GameControlEvent::kBack:
                    strcpy(out, prevValue);
                    menu_set_value(menu, editedItemName, prevValue);
                    menuMode = NORMAL;
                    break;
                default:
                    result = UNHANDLED;
                    break;
            }
            if (result == UNHANDLED) {
                switch (e.type) {
                    case SDL_KEYDOWN:
                    {
                        char ch = 0;
                        if (e.key.keysym.sym == SDLK_PERIOD)
                            ch = e.key.keysym.sym;
                        if (e.key.keysym.sym == SDLK_SLASH)
                            ch = e.key.keysym.sym;
                        if (e.key.keysym.sym == SDLK_MINUS)
                            ch = e.key.keysym.sym;
                        if (e.key.keysym.sym == SDLK_COLON)
                            ch = e.key.keysym.sym;
                        
                        if ((e.key.keysym.sym >= SDLK_KP0) && (e.key.keysym.sym <= SDLK_KP9))
                            ch = e.key.keysym.sym - SDLK_KP0 + '0';
                        if (e.key.keysym.sym == SDLK_KP_PERIOD)
                            ch = '.';
                        
                        if ((e.key.keysym.sym >= SDLK_0) && (e.key.keysym.sym <= SDLK_9))
                            ch = e.key.keysym.sym;
                        
                        if ((e.key.keysym.sym >= SDLK_a) && (e.key.keysym.sym <= SDLK_z))
                            ch = e.key.keysym.sym;
                        
                        if ((ch >= 'a') && (ch <= 'z')) {
                            if ((len == 0) || (out[len-1] == ' '))
                                ch = ch - 'a' + 'A';
                        }
                        
                        if (e.key.keysym.sym == SDLK_SPACE)
                            ch = ' ';
                        
                        if ((ch!=0) && (len < maxlen)) {
                            out[len++] = ch;
                            out[len]   = '_';
                            out[len+1] = 0;
                        }
                        
                        if ((e.key.keysym.sym == SDLK_BACKSPACE) && (len > 0))
                        {
                            out[len] = 0;
                            out[--len] = '_';
                        }
                        menu_set_value(menu, editedItemName, out, 0);
                    }
                    result = DONE;
                    break;
                }
            }
            break;
        default:
            switch (controlEvent.cursorEvent) {
                case GameControlEvent::kDown:
                    menu_next_item (menu);
                    break;
                case GameControlEvent::kUp:
                    menu_prev_item (menu);
                    break;
                case GameControlEvent::kStart:
                    switch (itemNames[menu_get_active_item(menu)]->type) {
                        case PuyoMenuItem::EDIT:
                            menuMode = EDITION;
                            editedItemName = itemNames[menu_get_active_item(menu)]->name;
                            len = 0;
                            maxlen = 32;
                            out[0] = '_';
                            out[1] = 0;
                            strcpy(prevValue, menu_get_value (menu, editedItemName));
                            menu_set_value(menu, editedItemName, "_");
                            break;
                        default:
                            menu_validate (menu);
                            itemSelected(menu_get_active_item(menu));
                            break;
                    }
                    break;
                default:
                    result = UNHANDLED;
                    break;
            }
            break;
    }
    return result;
}

const char *PuyoMenu::getValue(int itemNumber) const
{
    return menu_get_value(menu, itemNames[itemNumber]->name);
}

void PuyoMenu::addItem(const char *itemString, const char *itemValue, bool itemInactive, PuyoMenuItem::PuyoMenuItemType type)
{
    PuyoMenuItem *newItem = new PuyoMenuItem;
    newItem->name = itemString;
    if (itemValue != NULL)
        newItem->value = strdup(itemValue);
    else
        newItem->value = NULL;
    newItem->inactive = (itemInactive ? 1 : 0);
    newItem->type = type;
    itemNames.add(newItem);
}

void PuyoMenu::addSeparator()
{
    addItem("---", NULL, true);
}
*/
