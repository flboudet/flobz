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

#include "LevelThemeMenu.h"
#include "preferences.h"
#include "PuyoStrings.h"
#include "AnimatedPuyoTheme.h"

class LevelThemeSelectedAction : public Action {
public:
    LevelThemeSelectedAction(LevelThemePreview &previewWidget, String themeName);
    void action();
private:
    LevelThemePreview &previewWidget;
    String themeName;
};

LevelThemeSelectedAction::LevelThemeSelectedAction(LevelThemePreview &previewWidget, String themeName)
    : previewWidget(previewWidget), themeName(themeName)
{
}

void LevelThemeSelectedAction::action()
{
    getPuyoThemeManger()->setPreferedPuyoLevelTheme(themeName);
    previewWidget.themeSelected(themeName);
}


LevelThemeSelectionBox::LevelThemeSelectionBox(LevelThemePreview &themePreview)
    : themePreview(themePreview)
{
}

LevelThemeSelectionBox::~LevelThemeSelectionBox()
{
    int size = buttonList.size();
    for (int i = 0; i < size; i++)
    {
        delete buttonList[i];
        delete actionList[i];
    }
}

void LevelThemeSelectionBox::build()
{
    AdvancedBuffer<const char *> * themes = 
        getPuyoThemeManger()->getPuyoLevelThemeList();
    String pref = getPuyoThemeManger()->getPreferedPuyoLevelThemeName();
    int size = themes->size();
    for (int i = 0; i < size; i++)
    {
        Action * a = new LevelThemeSelectedAction(themePreview, (*themes)[i]);
        Button * b = new Button((*themes)[i], a);
        buttonList.add(b);
        actionList.add(a);
        add(b);
        if (pref == (*themes)[i])
        {
            focus(b);
            themePreview.themeSelected(pref);
        }
    }
}


/*****************************************************************************/

#define ONELEVELX (128.)
#define ONELEVELY (96.)

LevelThemePicturePreview::LevelThemePicturePreview()
{
      setPreferedSize(Vec3(ONELEVELX, ONELEVELY, 1.0));
      offsetX = offsetY = 0.;
}

LevelThemePicturePreview::~LevelThemePicturePreview()
{
      if (lilback != NULL) IIM_Free(lilback);
}

void LevelThemePicturePreview::draw(SDL_Surface *screen)
{
    if (lilback != NULL)
    {
      IIM_Rect r;
      Vec3 size = getSize();
      r.x = (Sint16)(getPosition().x+(size.x-ONELEVELX)/2.0);
      r.y = (Sint16)(getPosition().y+(size.y-ONELEVELY)/2.0);
      IIM_BlitSurface(lilback, NULL, screen, &r);
    }
}

void LevelThemePicturePreview::themeSelected(PuyoLevelTheme *  theme)
{
    if (theme != NULL)
    {
      if (lilback != NULL) IIM_Free(lilback);
      IIM_Rect r;
      // TODO : Draw the oponent.
      // Background
      IIM_Surface * templilback = iim_surface_duplicate(theme->getBackground());
      // Grids
      IIM_Surface * grid = theme->getGrid();
      r.x = 21;
      r.y = -1;
      r.w = grid->w;
      r.h = grid->h;
      IIM_BlitSurface(grid, NULL, templilback->surf, &r);
      r.x = 407;
      r.y = -1;
      IIM_BlitSurface(grid, NULL, templilback->surf, &r);
      // Speed meter
      IIM_Surface * speedFront = theme->getSpeedMeter(true);
      IIM_Surface * speedBack  = theme->getSpeedMeter(false);
      r.x = theme->getSpeedMeterX() - speedBack->w / 2;
      r.y = theme->getSpeedMeterY() - speedBack->h;
      r.w = speedBack->w;
      r.h = speedBack->h;
      IIM_Rect r2;
      IIM_BlitSurface(speedBack, NULL, templilback->surf, &r);
      r2.x = 0;
      r2.y = speedFront->h/2;
      r2.w = speedFront->w;
      r2.h = speedFront->h/2;
      r.y += speedFront->h/2;
      IIM_BlitSurface(speedFront, &r2, templilback->surf, &r);
	  // Final stretch
      lilback = iim_surface_resize(templilback,(int)ONELEVELX,(int)ONELEVELY);
    }
}

void LevelThemePicturePreview::idle(double currentTime) { }


/*****************************************************************************/

#define MARGIN (10.)

LevelThemePreview::LevelThemePreview() {}

void LevelThemePreview::build() {
    add(&name);
    add(&author);
    add(&picture);
    add(&description);
}

LevelThemePreview::~LevelThemePreview() {}

void LevelThemePreview::themeSelected(String themeName)
{
    PuyoLevelTheme * curTheme = getPuyoThemeManger()->getPuyoLevelTheme(themeName);
    name.setFont(GameUIDefaults::FONT_TEXT);
    name.setValue(themeName);
    author.setFont(GameUIDefaults::FONT_SMALL_INFO);
    author.setValue(curTheme->getAuthor());
    description.setFont(GameUIDefaults::FONT_SMALL_INFO);
    description.setValue(curTheme->getComments());
    picture.themeSelected(curTheme);
    Vec3 marges(0.0,MARGIN,0.0);
    setPreferedSize(name.getPreferedSize()+marges
        +author.getPreferedSize()+marges
        +description.getPreferedSize()+marges
        +picture.getPreferedSize());
    if (parent)
      parent->arrangeWidgets();
}


/*****************************************************************************/

LevelThemeMenu::LevelThemeMenu(PuyoRealMainScreen *mainScreen)
    : PuyoMainScreenMenu(mainScreen), popAction(mainScreen),
      themeMenuTitle("Select Level:"),
      themePreview(), themeList(themePreview)
{
}

void LevelThemeMenu::build() {
    themeList.build();
    themePreview.build();

    add(&themeMenuTitle);
    add(&themeList);
    add(&themePreview);
}
