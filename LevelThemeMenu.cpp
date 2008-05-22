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

LevelThemeSelectionBox::LevelThemeSelectionBox()
    : themePreview(), Spacer0(), Spacer1(), Spacer2(), Spacer3()
{
    leftArrow = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/leftarrow.png"));
    rightArrow = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/rightarrow.png"));

    prevButton = new Image(leftArrow);
    nextButton = new Image(rightArrow);
    
    Spacer0.setPreferedSize(Vec3(15.0f, 0.0f));
    Spacer1.setPreferedSize(Vec3(0.0f, 0.0f));
    Spacer2.setPreferedSize(Vec3(0.0f, 0.0f));
    Spacer3.setPreferedSize(Vec3(15.0f, 0.0f));
    
    setPolicy(USE_MAX_SIZE_NO_MARGIN);
}

LevelThemeSelectionBox::~LevelThemeSelectionBox()
{
    delete prevButton;
    delete nextButton;
    IIM_Free(leftArrow);
    IIM_Free(rightArrow);
}

void LevelThemeSelectionBox::build()
{
    themePreview.build();
    AdvancedBuffer<const char *> * themes = getPuyoThemeManger()->getPuyoLevelThemeList();
    String pref = getPuyoThemeManger()->getPreferedPuyoLevelThemeName();
    int size = themes->size();
    bool found = false;
    for (int i = 0; i < size; i++)
    {
        if (pref == (*themes)[i])
        {
            themePreview.themeSelected(pref);
            found = true;
        }
    }
    
    add(&Spacer0);
    
    prevButton->setFocusable(size > 1);
    prevButton->setOnStartAction(this);
    prevButton->setInvertedFocus(true);
    add(prevButton);
    
    add(&Spacer1);
    if (found == false && size > 0)
    {
        themePreview.themeSelected((*themes)[0]);
        getPuyoThemeManger()->setPreferedPuyoLevelTheme((*themes)[0]);
    }
    add(&themePreview);
    add(&Spacer2);

    nextButton->setFocusable(size > 1);
    nextButton->setOnStartAction(this);
    nextButton->setInvertedFocus(true);
    add(nextButton);
    
    add(&Spacer3);
}

void LevelThemeSelectionBox::action(Widget *sender, int actionType, GameControlEvent *event)
{
    AdvancedBuffer<const char *> * themes = getPuyoThemeManger()->getPuyoLevelThemeList();
    String pref = getPuyoThemeManger()->getPreferedPuyoLevelThemeName();
    int size = themes->size();
    if (size <= 0) return;
    
    int currentTheme;
    // get the selected theme id or zero if the prefered theme if not there
    for (currentTheme = size-1; currentTheme > 0; --currentTheme)
    {
        if (pref == (*themes)[currentTheme]) break;
    }
    if (sender == prevButton) {
        (currentTheme <= 0) ? currentTheme = size - 1 : currentTheme--;
        themePreview.themeSelected((*themes)[currentTheme]);
        getPuyoThemeManger()->setPreferedPuyoLevelTheme((*themes)[currentTheme]);
    }
    else if (sender == nextButton) {
        currentTheme = (currentTheme+1)%size;
        themePreview.themeSelected((*themes)[currentTheme]);
        getPuyoThemeManger()->setPreferedPuyoLevelTheme((*themes)[currentTheme]);
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
      themeList()
{
}

void LevelThemeMenu::build() {
    setPolicy(USE_MIN_SIZE);
    screenTitleFrame.setPreferedSize(Vec3(0, 20));
    screenTitleFrame.add(&themeMenuTitle);
    add(&screenTitleFrame);
    buttonsBox.add(&themeList);
    buttonsBox.add(&backButton);
    add(&buttonsBox);
    themeList.build();
}


