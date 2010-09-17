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

#include "ThemeMenu.h"
#include "preferences.h"
#include "PuyoStrings.h"
#include "Theme.h"

using namespace event_manager;

PuyoThemeSelectionBox::PuyoThemeSelectionBox()
    : themePreview(), Spacer0(), Spacer1(), Spacer2(), Spacer3()
{
    prevButton = new Image(theCommander->getLeftArrow());
    nextButton = new Image(theCommander->getRightArrow());

    Spacer0.setPreferedSize(Vec3(15.0f, 0.0f));
    Spacer1.setPreferedSize(Vec3(0.0f, 0.0f));
    Spacer2.setPreferedSize(Vec3(0.0f, 0.0f));
    Spacer3.setPreferedSize(Vec3(15.0f, 0.0f));

    setPolicy(USE_MAX_SIZE_NO_MARGIN);
}

PuyoThemeSelectionBox::~PuyoThemeSelectionBox()
{
    delete prevButton;
    delete nextButton;
}

void PuyoThemeSelectionBox::build()
{
    themePreview.build();
    const std::vector<std::string> &themes = theCommander->getPuyoSetThemeList();
    int size = themes.size();
    std::string preferedTheme = themes[0]; // TODO: get from prefs
    // Select the prefered theme
    bool found = false;
    for (std::vector<std::string>::const_iterator iter = themes.begin() ;
         (iter != themes.end()) && (!found); iter ++) {
        themePreview.themeSelected(preferedTheme);
        found = true;
    }
    if (!found) {
        themePreview.themeSelected(themes[0]);
        // TODO: set in prefs
    }
    add(&Spacer0);
    prevButton->setFocusable(size > 1);
    prevButton->setOnAction(this);
    prevButton->setInvertedFocus(true);
    add(prevButton);
    add(&Spacer1);
    add(&themePreview);
    add(&Spacer2);
    nextButton->setFocusable(size > 1);
    nextButton->setOnAction(this);
    nextButton->setInvertedFocus(true);
    add(nextButton);
    add(&Spacer3);
}

void PuyoThemeSelectionBox::action(Widget *sender, int actionType, GameControlEvent *event)
{
#ifdef DISABLED
    if (!event->isUp) return;
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
#endif
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

void PuyoThemePicturePreview::draw(DrawTarget *dt)
{
    if (curTheme != NULL)
    {
      IosRect r;
      Vec3 size = getSize();
      r.x = (int16_t)(getPosition().x+(size.x-NUMBER_OF_PUYOS*ONEPUYO+(NUMBER_OF_PUYOS-1)*ONEPUYO/4.0)/2.0);
      r.y = (int16_t)(getPosition().y+(size.y-ONEPUYO)/2.0);
      r.h = 0;
      r.w = 0;
      for (int i=0; i<NUMBER_OF_PUYOS; i++)
      {
        IosRect rect = r;
        rect.x += (int16_t)((i*3*ONEPUYO)/4);
        const PuyoTheme &t = curTheme->getPuyoTheme((PuyoState)(PUYO_BLUE+i));
        dt->draw(t.getShadowSurface(), NULL, &rect);
        dt->draw(t.getPuyoSurfaceForValence(0), NULL, &rect);
        dt->draw(t.getEyeSurfaceForIndex(imageForIndex(eyes[i])), NULL, &rect);
      }
    }
}

void PuyoThemePicturePreview::themeSelected(PuyoSetTheme *  theme)
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

void PuyoThemePreview::themeSelected(const std::string &themeName)
{
#define _ComputeVZoneSize(A,B) Vec3(A.x>B.x?A.x:B.x,A.y+B.y+GameUIDefaults::SPACING,1.0)
    PuyoSetThemeRef curTheme = theCommander->getPuyoSetTheme(themeName.c_str());
    name.setFont(GameUIDefaults::FONT_TEXT);
    name.setValue(curTheme->getLocalizedName().c_str());
    author.setFont(GameUIDefaults::FONT_SMALL_INFO);
    author.setValue(curTheme->getAuthor().c_str());
    description.setFont(GameUIDefaults::FONT_SMALL_INFO);
    description.setValue(curTheme->getComments().c_str());
    picture.themeSelected(curTheme);
    Vec3 marges(0.0,MARGIN,0.0);
    Vec3 one=_ComputeVZoneSize(name.getPreferedSize(),author.getPreferedSize());
    one=_ComputeVZoneSize(one,description.getPreferedSize());
    setPreferedSize(_ComputeVZoneSize(one,picture.getPreferedSize()));
    if (parent)
      parent->arrangeWidgets();
}


/*****************************************************************************/

ThemeMenu::ThemeMenu(MainScreen *mainScreen)
    : MainScreenMenu(mainScreen),
      screenTitleFrame(theCommander->getSeparatorFramePicture()),
      themeMenuTitle(theCommander->getLocalizedString("Puyo theme")), popAction(mainScreen),
      backButton(theCommander->getLocalizedString("Back"), &popAction),
      themeList()
{
}

void ThemeMenu::build() {
    setPolicy(USE_MIN_SIZE);
    screenTitleFrame.setPreferedSize(Vec3(0, 20));
    screenTitleFrame.add(&themeMenuTitle);
    add(&screenTitleFrame);
    buttonsBox.add(&themeList);
    buttonsBox.add(&backButton);
    add(&buttonsBox);
    themeList.build();
}
