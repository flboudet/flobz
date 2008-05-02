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
    AdvancedBuffer<const char *> * themes = getPuyoThemeManger()->getPuyoLevelThemeList();
    AdvancedBuffer<PuyoLevelTheme *> * themesObjects = getPuyoThemeManger()->getPuyoLevelThemeObjectList();
    String pref = getPuyoThemeManger()->getPreferedPuyoLevelThemeName();
    int size = themes->size();
    for (int i = 0; i < size; i++)
    {
        Action * a = new LevelThemeSelectedAction(themePreview, (*themes)[i]);
        Button * b = new Button((*themesObjects)[i]->getLocalizedName(), a);
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
      offsetX = offsetY = 0.;
      shouldRecache = true;
      shouldResize = true;
      curTheme = NULL;
      picture = lilback = NULL;
      setPreferedSize(Vec3(ONELEVELX, ONELEVELY, 1.0));
}

LevelThemePicturePreview::~LevelThemePicturePreview()
{
      if (lilback != NULL) IIM_Free(lilback);
      if (picture != NULL) IIM_Free(picture);
}

void LevelThemePicturePreview::draw(SDL_Surface *screen)
{
	updatePicture();
    if (lilback != NULL)
    {
      IIM_Rect r;
      Vec3 size = getSize();
      Vec3 pos = getPosition();
      r.x = (Sint16)(pos.x+offsetX);
      r.y = (Sint16)(pos.y+offsetY);
      r.w = (Sint16)(size.x);
      r.h = (Sint16)(size.y);
      IIM_BlitSurface(lilback, NULL, screen, &r);
    }
}

void LevelThemePicturePreview::updatePicture(void)
{
    if ((curTheme != NULL) && (shouldRecache == true))
    {
      if (picture != NULL) IIM_Free(picture);
      IIM_Rect r;
      // TODO : Draw the oponent.
      // Background
      picture = iim_surface_duplicate(curTheme->getBackground());
      // Grids
      IIM_Surface * grid = curTheme->getGrid();
      if (grid != NULL) {
	r.x = 21;
	r.y = -1;
	r.w = grid->w;
	r.h = grid->h;
	IIM_BlitSurface(grid, NULL, picture->surf, &r);
	r.x = 407;
	r.y = -1;
	IIM_BlitSurface(grid, NULL, picture->surf, &r);
      }
      // Speed meter
      IIM_Surface * speedFront = curTheme->getSpeedMeter(true);
      IIM_Surface * speedBack  = curTheme->getSpeedMeter(false);
      r.x = curTheme->getSpeedMeterX() - speedBack->w / 2;
      r.y = curTheme->getSpeedMeterY() - speedBack->h;
      r.w = speedBack->w;
      r.h = speedBack->h;
      IIM_Rect r2;
      IIM_BlitSurface(speedBack, NULL, picture->surf, &r);
      r2.x = 0;
      r2.y = speedFront->h/2;
      r2.w = speedFront->w;
      r2.h = speedFront->h/2;
      r.y += speedFront->h/2;
      IIM_BlitSurface(speedFront, &r2, picture->surf, &r);
      shouldRecache = false;
      shouldResize = true;
    }
    
    if ((shouldResize) && (picture!=NULL))
    {
      if (lilback != NULL) IIM_Free(lilback);
	  Vec3 s=getSize();
      lilback = iim_surface_resize(picture,(int)s.x,(int)s.y);
      shouldResize = false;
    }
}

void LevelThemePicturePreview::themeSelected(PuyoLevelTheme * theme)
{
    if (theme != NULL)
    {
      curTheme = theme;
      shouldRecache = true;
    }
}

void LevelThemePicturePreview::setSize(const Vec3 &v3)
{
	Vec3 s=v3;
	if ((s.x/s.y) < (ONELEVELX/ONELEVELY))
	{
		s.y=s.x*ONELEVELY/ONELEVELX;
		offsetX=0.0; offsetY=(v3.y-s.y)/2.0;
	}
	else
	{
		s.x=s.y*ONELEVELX/ONELEVELY;
		offsetY=0.0; offsetX=(v3.x-s.x)/2.0;
	}
	if ((getSize().x != s.x) || (getSize().y != s.y))
	{
    	shouldResize = true;
		Widget::setSize(s);
	}
}

void LevelThemePicturePreview::idle(double currentTime) { }


/*****************************************************************************/

LevelThemePreview::LevelThemePreview() {}

void LevelThemePreview::build() {
    name.setFont(GameUIDefaults::FONT_TEXT);
    //author.setFont(GameUIDefaults::FONT_SMALL_INFO);
    description.setFont(GameUIDefaults::FONT_SMALL_INFO);
    add(&name);
    //add(&author);
    add(&picture);
    add(&description);
}

LevelThemePreview::~LevelThemePreview() {}

void LevelThemePreview::themeSelected(String themeName)
{
#define _ComputeVZoneSize(A,B) Vec3(A.x>B.x?A.x:B.x,A.y+B.y+GameUIDefaults::SPACING,1.0)
    PuyoLevelTheme * curTheme = getPuyoThemeManger()->getPuyoLevelTheme(themeName);
    if (curTheme->getAuthor() == "iOS-Software") name.setValue(curTheme->getLocalizedName());
    else name.setValue(themeName+" ("+curTheme->getAuthor()+")");
    //author.setValue(curTheme->getAuthor());
    description.setValue(curTheme->getComments());
    Vec3 one=_ComputeVZoneSize(name.getPreferedSize(),description.getPreferedSize());
    setPreferedSize(_ComputeVZoneSize(one,picture.getPreferedSize()));
    if (parent)
      parent->arrangeWidgets();
    picture.themeSelected(curTheme);
}

/*****************************************************************************/

LevelThemeMenu::LevelThemeMenu(PuyoMainScreen *mainScreen)
    : PuyoMainScreenMenu(mainScreen),
      screenTitleFrame(theCommander->getSeparatorFramePicture()),
      themeMenuTitle(theCommander->getLocalizedString("Level theme")), popAction(mainScreen),
      backButton(theCommander->getLocalizedString("Back"), &popAction), 
      themePreview(), themeList(themePreview)
{
}

void LevelThemeMenu::build() {
    setPolicy(USE_MIN_SIZE);
    screenTitleFrame.setPreferedSize(Vec3(0, 20));
    screenTitleFrame.add(&themeMenuTitle);
    add(&screenTitleFrame);
    buttonsBox.add(&themeList);
    buttonsBox.add(&themePreview);
    buttonsBox.add(&backButton);
    add(&buttonsBox);
    themePreview.build();
    themeList.build();
}
