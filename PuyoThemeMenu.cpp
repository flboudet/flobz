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

PuyoThemeSelectionBox::PuyoThemeSelectionBox()
    : themePreview(), Spacer1(), Spacer2()
{
    leftArrow = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/leftarrow.png"));
    rightArrow = IIM_Load_Absolute_DisplayFormatAlpha(theCommander->getDataPathManager().getPath("gfx/rightarrow.png"));
    
    prevButton = new Image(leftArrow);
    nextButton = new Image(rightArrow);
    
    Spacer1.setPreferedSize(Vec3(10.0f, 0.0f));
    Spacer2.setPreferedSize(Vec3(10.0f, 0.0f));
    
    setPolicy(USE_MAX_SIZE_NO_MARGIN);
}

PuyoThemeSelectionBox::~PuyoThemeSelectionBox()
{
    delete prevButton;
    delete nextButton;
    IIM_Free(leftArrow);
    IIM_Free(rightArrow);
}

void PuyoThemeSelectionBox::build()
{
    themePreview.build();
    AdvancedBuffer<const char *> * themes = getPuyoThemeManger()->getAnimatedPuyoSetThemeList();
    String pref = getPuyoThemeManger()->getPreferedAnimatedPuyoSetThemeName();
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
    
    add(&Spacer1);
    
    prevButton->setFocusable(size > 1);
    prevButton->setOnStartAction(this);
    prevButton->setInvertedFocus(true);
    add(prevButton);
    
    if (found == false && size > 0)
    {
        themePreview.themeSelected((*themes)[0]);
        getPuyoThemeManger()->setPreferedAnimatedPuyoSetTheme((*themes)[0]);
    }
    add(&themePreview);
    
    nextButton->setFocusable(size > 1);
    nextButton->setOnStartAction(this);
    nextButton->setInvertedFocus(true);
    add(nextButton);
    
    add(&Spacer2);
}

void PuyoThemeSelectionBox::action(Widget *sender, int actionType, GameControlEvent *event)
{
    AdvancedBuffer<const char *> * themes = getPuyoThemeManger()->getAnimatedPuyoSetThemeList();
    String pref = getPuyoThemeManger()->getPreferedAnimatedPuyoSetThemeName();
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
        getPuyoThemeManger()->setPreferedAnimatedPuyoSetTheme((*themes)[currentTheme]);
    }
    else if (sender == nextButton) {
        currentTheme = (currentTheme+1)%size;
        themePreview.themeSelected((*themes)[currentTheme]);
        getPuyoThemeManger()->setPreferedAnimatedPuyoSetTheme((*themes)[currentTheme]);
    }
}


/*****************************************************************************/

#define ONEPUYO (32.)

PuyoThemePicturePreview::PuyoThemePicturePreview()
{
      setPreferedSize(Vec3(NUMBER_OF_PUYOS*ONEPUYO-(NUMBER_OF_PUYOS-1)*ONEPUYO/4.0, ONEPUYO, 1.0));
      offsetX = offsetY = 0.;
      curTheme = NULL;
      for (int i=0; i<NUMBER_OF_PUYOS; i++)
      {
        eyes[i] = 0;
      }
      lastTime = 0.;
}

static int imageForIndex(int i)
{
    if (i<NUMBER_OF_PUYO_EYES) return i;
    else return 2*(NUMBER_OF_PUYO_EYES-1)-i;
}

void PuyoThemePicturePreview::draw(SDL_Surface *screen)
{
    if (curTheme != NULL)
    {
      IIM_Rect r;
      Vec3 size = getSize();
      r.x = (Sint16)(getPosition().x+(size.x-NUMBER_OF_PUYOS*ONEPUYO+(NUMBER_OF_PUYOS-1)*ONEPUYO/4.0)/2.0);
      r.y = (Sint16)(getPosition().y+(size.y-ONEPUYO)/2.0);
      r.h = 0;
      r.w = 0;
      for (int i=0; i<NUMBER_OF_PUYOS; i++)
      {
        IIM_Rect rect = r;
        rect.x += (Sint16)((i*3*ONEPUYO)/4);
        AnimatedPuyoTheme * t = curTheme->getAnimatedPuyoTheme((PuyoState)(PUYO_BLUE+i));
        IIM_BlitSurface(t->getSurface(PUYO_SHADOWS,0), NULL, screen, &rect);
        IIM_BlitSurface(t->getSurface(PUYO_FACES,0), NULL, screen, &rect);
        IIM_BlitSurface(t->getSurface(PUYO_EYES,imageForIndex(eyes[i])), NULL, screen, &rect);
      }
    }
}

void PuyoThemePicturePreview::themeSelected(AnimatedPuyoSetTheme *  theme)
{
    curTheme = theme;
    //requestDraw();
}

void PuyoThemePicturePreview::idle(double currentTime)
{
    bool refresh = false;

    if ((currentTime - lastTime) > 0.1)
    {
      //fprintf(stderr,"TIME! %d\n",imageForIndex(eyes[0]));
      for (int i=0; i<NUMBER_OF_PUYOS; i++)
      {
        if(eyes[i]>0)
        {
            refresh = true;
            eyes[i]++;
            if (eyes[i] > 2*(NUMBER_OF_PUYO_EYES-1)) eyes[i] = 0;
        }
        else 
        {
            if ((random() % 50) == 0)
            {
              refresh = true;
              eyes[i] = 1;
            }
        }
      }
      lastTime = currentTime;
    }
    if (refresh) requestDraw();
}


/*****************************************************************************/

#define MARGIN (10.)

PuyoThemePreview::PuyoThemePreview() {}

void PuyoThemePreview::build() {
    add(&name);
    add(&author);
    add(&picture);
    add(&description);
}

PuyoThemePreview::~PuyoThemePreview() {}

void PuyoThemePreview::themeSelected(String themeName)
{
#define _ComputeVZoneSize(A,B) Vec3(A.x>B.x?A.x:B.x,A.y+B.y+GameUIDefaults::SPACING,1.0)
    AnimatedPuyoSetTheme * curTheme = getPuyoThemeManger()->getAnimatedPuyoSetTheme(themeName);
    name.setFont(GameUIDefaults::FONT_TEXT);
    name.setValue(curTheme->getLocalizedName());
    author.setFont(GameUIDefaults::FONT_SMALL_INFO);
    author.setValue(curTheme->getAuthor());
    description.setFont(GameUIDefaults::FONT_SMALL_INFO);
    description.setValue(curTheme->getComments());
    picture.themeSelected(curTheme);
    Vec3 marges(0.0,MARGIN,0.0);
    Vec3 one=_ComputeVZoneSize(name.getPreferedSize(),author.getPreferedSize());
    one=_ComputeVZoneSize(one,description.getPreferedSize());
    setPreferedSize(_ComputeVZoneSize(one,picture.getPreferedSize()));
    if (parent)
      parent->arrangeWidgets();
}


/*****************************************************************************/

PuyoThemeMenu::PuyoThemeMenu(PuyoMainScreen *mainScreen)
    : PuyoMainScreenMenu(mainScreen),
      screenTitleFrame(theCommander->getSeparatorFramePicture()),
      themeMenuTitle(theCommander->getLocalizedString("Puyo theme")), popAction(mainScreen),
      backButton(theCommander->getLocalizedString("Back"), &popAction),
      themeList()
{
}

void PuyoThemeMenu::build() {
    setPolicy(USE_MIN_SIZE);
    screenTitleFrame.setPreferedSize(Vec3(0, 20));
    screenTitleFrame.add(&themeMenuTitle);
    add(&screenTitleFrame);
    buttonsBox.add(&themeList);
    buttonsBox.add(&backButton);
    add(&buttonsBox);
    themeList.build();
}
