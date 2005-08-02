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

#include "PuyoThemeMenu.h"
#include "preferences.h"
#include "PuyoStrings.h"
#include "AnimatedPuyoTheme.h"

class PuyoThemeSelectedAction : public Action {
public:
    PuyoThemeSelectedAction(PuyoThemePreview &previewWidget, String themeName);
    void action();
private:
    PuyoThemePreview &previewWidget;
    String themeName;
};

PuyoThemeSelectedAction::PuyoThemeSelectedAction(PuyoThemePreview &previewWidget, String themeName)
    : previewWidget(previewWidget), themeName(themeName)
{
}

void PuyoThemeSelectedAction::action()
{
    previewWidget.themeSelected(themeName);
    getPuyoThemeManger()->setPreferedAnimatedPuyoSetTheme(themeName);
}


PuyoThemeSelectionBox::PuyoThemeSelectionBox(PuyoThemePreview &themePreview)
    : themePreview(themePreview)
{
}

PuyoThemeSelectionBox::~PuyoThemeSelectionBox()
{
    int size = buttonList.size();
    for (int i = 0; i < size; i++)
    {
        delete buttonList[i];
        delete actionList[i];
    }
}

void PuyoThemeSelectionBox::build()
{
    AdvancedBuffer<const char *> * themes = 
        getPuyoThemeManger()->getAnimatedPuyoSetThemeList();
    int size = themes->size();
    for (int i = 0; i < size; i++)
    {
        Action * a = new PuyoThemeSelectedAction(themePreview, (*themes)[i]);
        Button * b = new Button((*themes)[i], a);
        buttonList.add(b);
        actionList.add(a);
        add(b);
    }
}

PuyoThemePreview::PuyoThemePreview() : Text("PREVIEW")
{
}

void PuyoThemePreview::themeSelected(String themeName)
{
    setValue(themeName);
}

PuyoThemeMenu::PuyoThemeMenu(PuyoRealMainScreen *mainScreen)
    : PuyoMainScreenMenu(mainScreen), popAction(mainScreen),
      backButton("Back", &popAction), themeMenuTitle("Change Theme..."),
      themePreview(), themeList(themePreview)
{
}

void PuyoThemeMenu::build() {
    themeList.build();

    add(&themeMenuTitle);
    add(&themeList);
    add(&themePreview);
    add(&backButton);
}
