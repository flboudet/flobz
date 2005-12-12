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

#ifndef PUYO_THEME_MENU_H
#define PUYO_THEME_MENU_H

#include "gameui.h"
#include "PuyoCommander.h"
#include "AnimatedPuyoTheme.h"

class PuyoThemePicturePreview : public Widget, public IdleComponent {
    public:
        PuyoThemePicturePreview();

        // Implements IdleComponent
        virtual void idle(double currentTime);
        virtual IdleComponent *getIdleComponent() { return this; }

        void build();
        void themeSelected(AnimatedPuyoSetTheme * theme);

    protected:
      void draw(SDL_Surface *screen);
      SoFont *font;
      
    private:
      String label;
      AnimatedPuyoSetTheme * curTheme;
      double offsetX;
      double offsetY;
      int eyes[NUMBER_OF_PUYOS];
      double lastTime;
};

class PuyoThemePreview : public VBox {
    public:
        PuyoThemePreview();
        virtual ~PuyoThemePreview();
        
        void build();
        void themeSelected(String themeName);

    private:
        Text name;
        Text author;
        PuyoThemePicturePreview picture;
        Text description;
};
/*

class ScrollList : public HBox {
public:
    ScrollList();
    virtual ~ScrollList();
    void add(String &name);
    void select(String &name);
    void selectPrevious();
    void selectNext();
private:
    int selected;
    int maxShown;
    Button * previous;
    Button * next;
    Action * previousAction;
    Action * nextAction;
    AdvancedBuffer<Text *> elementList;
    AdvancedBuffer<Action *> actionList;
};

ScrollList::ScrollList() :
{
    maxShown = 3;
    selected = -1;
    previousAction = new ???(this,???);
    previous = new Button("<",previousAction);
    add(previous);
    nextAction = new ???(this,???);
    next = new Button(">",nextAction);
    add(next);
}

ScrollList::~ScrollList()
{
    delete previous;
    delete previousAction;
    delete next;
    delete nextAction;
    int i, size = elementList.size();
    for (i=0; i<size; i++)
    {
        delete elementList[i];
        //delete actionList[i]; should we?
    }
}

void ScrollList::add(String &name, Action &action)
{
    text = new Text(name,GameUIDefaults::FONT_INACTIVE);
    elementList.add(&name);
    actionList.add(&action);
    if (selected < 0) selected = 0;
}

void ScrollList::build()
{
    int i, size = elementList.size();
    for (i=0; (i<maxShown) && (i<size); i++)
    {
        itemID = (selected-(maxShown/2)+i+size)%size;
        changeChild(i+1,elementList[itemID]);
        elementList[itemID]->setFont( (selected == itemID) ?
         GameUIDefaults::FONT :
         GameUIDefaults::FONT_INACTIVE );
    }
}

void ScrollList::selectPrevious()
{
    if (elementList.size() <= 0) return;
    selected = (selected - 1) % elementList.size();
    build();
    actionList[selected]->action();
}
void ScrollList::selectNext()
{
    if (elementList.size() <= 0) return;
    selected = (selected + 1) % elementList.size();
    build();
    actionList[selected]->action();
}
void ScrollList::select(String &name)
{
    int i, size = elementList.size();
    for (i=0; i<size; i++)
    {
        if (*elementList[i] == name)
        {
            selected = i;
            actionList[selected]->action();
            break;
        }
    }
}
*/
class PuyoThemeSelectionBox : public HBox {
public:
    PuyoThemeSelectionBox(PuyoThemePreview &themePreview);
    virtual ~PuyoThemeSelectionBox();
    void build();
private:
    PuyoThemePreview &themePreview;
    AdvancedBuffer<Button *> buttonList;
    AdvancedBuffer<Action *> actionList;
};

class PuyoThemeMenu : public PuyoMainScreenMenu {
public:
    PuyoThemeMenu(PuyoRealMainScreen *mainScreen);
    void build();
private:
    Text themeMenuTitle;
    PuyoPopMenuAction popAction;
    Button backButton;
    PuyoThemePreview themePreview;
    PuyoThemeSelectionBox themeList;
};

#endif
