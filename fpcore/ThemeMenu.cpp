/* FloboPop
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
#include "FPStrings.h"
#include "Theme.h"

using namespace event_manager;

FloboThemeSelectionBox::FloboThemeSelectionBox()
    : _themePreview(), _Spacer0(), _Spacer1(), _Spacer2(), _Spacer3()
{
    _prevButton = new Image(theCommander->getLeftArrow());
    _nextButton = new Image(theCommander->getRightArrow());

    _Spacer0.setPreferedSize(Vec3(15.0f, 0.0f));
    _Spacer1.setPreferedSize(Vec3(0.0f, 0.0f));
    _Spacer2.setPreferedSize(Vec3(0.0f, 0.0f));
    _Spacer3.setPreferedSize(Vec3(15.0f, 0.0f));

    setPolicy(USE_MAX_SIZE_NO_MARGIN);
}

FloboThemeSelectionBox::~FloboThemeSelectionBox()
{
    delete _prevButton;
    delete _nextButton;
}

void FloboThemeSelectionBox::build()
{
    _themePreview.build();
    const std::vector<std::string> &themes = theCommander->getFloboSetThemeList();
    int size = themes.size();
    std::string preferedTheme = theCommander->getPreferedFloboSetThemeName();
    // Select the prefered theme
    bool found = false;
    for (std::vector<std::string>::const_iterator iter = themes.begin() ;
         (iter != themes.end()) && (!found); iter ++) {
        _themePreview.themeSelected(preferedTheme);
        found = true;
    }
    if (!found) {
        _themePreview.themeSelected(themes[0]);
        theCommander->setPreferedFloboSetThemeName(themes[0].c_str());
    }
    add(&_Spacer0);
    _prevButton->setFocusable(size > 1);
    _prevButton->setOnAction(this);
    _prevButton->setInvertedFocus(true);
    add(_prevButton);
    add(&_Spacer1);
    add(&_themePreview);
    add(&_Spacer2);
    _nextButton->setFocusable(size > 1);
    _nextButton->setOnAction(this);
    _nextButton->setInvertedFocus(true);
    add(_nextButton);
    add(&_Spacer3);
}

void FloboThemeSelectionBox::action(Widget *sender, int actionType, GameControlEvent *event)
{
    if (!event->isUp) return;
    const std::vector<std::string> &themes = theCommander->getFloboSetThemeList();
    std::string pref = theCommander->getPreferedFloboSetThemeName();
    int size = themes.size();
    if (size <= 0) return;

    int currentTheme;
    // get the selected theme id or zero if the prefered theme if not there
    for (currentTheme = size-1; currentTheme > 0; --currentTheme)
    {
        if (pref == themes[currentTheme]) break;
    }
    if (sender == _prevButton) {
        (currentTheme <= 0) ? currentTheme = size - 1 : currentTheme--;
        _themePreview.themeSelected(themes[currentTheme]);
        theCommander->setPreferedFloboSetThemeName(themes[currentTheme].c_str());
    }
    else if (sender == _nextButton) {
        currentTheme = (currentTheme+1)%size;
        _themePreview.themeSelected(themes[currentTheme]);
        theCommander->setPreferedFloboSetThemeName(themes[currentTheme].c_str());
    }
}


/*****************************************************************************/

#define ONEPUYO (32.)

FloboThemePicturePreview::FloboThemePicturePreview()
{
      setPreferedSize(Vec3(NUMBER_OF_FLOBOS*ONEPUYO-(NUMBER_OF_FLOBOS-1)*ONEPUYO/4.0, ONEPUYO, 1.0));
      _offsetX = _offsetY = 0.;
      _curTheme = NULL;
      for (int i=0; i<NUMBER_OF_FLOBOS; i++)
      {
        _eyes[i] = 0;
      }
      _lastTime = 0.;
}

static int imageForIndex(int i)
{
    if (i<NUMBER_OF_FLOBO_EYES) return i;
    else return 2*(NUMBER_OF_FLOBO_EYES-1)-i;
}

void FloboThemePicturePreview::draw(DrawTarget *dt)
{
    if (_curTheme != NULL)
    {
      IosRect r;
      Vec3 size = getSize();
      r.x = (int16_t)(getPosition().x+(size.x-NUMBER_OF_FLOBOS*ONEPUYO+(NUMBER_OF_FLOBOS-1)*ONEPUYO/4.0)/2.0);
      r.y = (int16_t)(getPosition().y+(size.y-ONEPUYO)/2.0);
      r.h = 32;
      r.w = 32;
      for (int i=0; i<NUMBER_OF_FLOBOS; i++)
      {
        IosRect rect = r;
        rect.x += (int16_t)((i*3*ONEPUYO)/4);
        const FloboTheme &t = _curTheme->getFloboTheme((FloboState)(FLOBO_BLUE+i));
        dt->draw(t.getShadowSurface(), NULL, &rect);
        dt->draw(t.getFloboSurfaceForValence(0), NULL, &rect);
        dt->draw(t.getEyeSurfaceForIndex(imageForIndex(_eyes[i])), NULL, &rect);
      }
    }
}

void FloboThemePicturePreview::themeSelected(FloboSetTheme *  theme)
{
    _curTheme = theme;
    //requestDraw();
}

void FloboThemePicturePreview::idle(double currentTime)
{
    bool refresh = false;

    if ((currentTime - _lastTime) > 0.1)
    {
      for (int i=0; i<NUMBER_OF_FLOBOS; i++)
      {
        if(_eyes[i]>0)
        {
            refresh = true;
            _eyes[i]++;
            if (_eyes[i] > 2*(NUMBER_OF_FLOBO_EYES-1)) _eyes[i] = 0;
        }
        else
        {
            if ((random() % 50) == 0)
            {
              refresh = true;
              _eyes[i] = 1;
            }
        }
      }
      _lastTime = currentTime;
    }
    if (refresh) requestDraw();
}


/*****************************************************************************/

#define MARGIN (10.)

FloboThemePreview::FloboThemePreview() {}

void FloboThemePreview::build() {
    add(&_name);
    add(&_author);
    add(&_picture);
    add(&_description);
}

FloboThemePreview::~FloboThemePreview() {}

void FloboThemePreview::themeSelected(const std::string &themeName)
{
#define _ComputeVZoneSize(A,B) Vec3(A.x>B.x?A.x:B.x,A.y+B.y+GameUIDefaults::SPACING,1.0)
    FloboSetThemeRef curTheme = theCommander->getFloboSetTheme(themeName.c_str());
    _name.setFont(GameUIDefaults::FONT_TEXT);
    _name.setValue(curTheme->getLocalizedName().c_str());
    _author.setFont(GameUIDefaults::FONT_SMALL_INFO);
    _author.setValue(curTheme->getAuthor().c_str());
    _description.setFont(GameUIDefaults::FONT_SMALL_INFO);
    _description.setValue(curTheme->getComments().c_str());
    _picture.themeSelected(curTheme);
    Vec3 marges(0.0,MARGIN,0.0);
    Vec3 one=_ComputeVZoneSize(_name.getPreferedSize(),_author.getPreferedSize());
    one=_ComputeVZoneSize(one,_description.getPreferedSize());
    setPreferedSize(_ComputeVZoneSize(one,_picture.getPreferedSize()));
    if (parent)
      parent->arrangeWidgets();
}


/*****************************************************************************/

ThemeMenu::ThemeMenu(MainScreen *mainScreen)
    : MainScreenMenu(mainScreen),
      _screenTitleFrame(theCommander->getSeparatorFramePicture()),
      _themeMenuTitle(theCommander->getLocalizedString("Flobo theme")), _popAction(mainScreen),
      _backButton(theCommander->getLocalizedString("Back"), &_popAction),
      _themeList()
{
}

void ThemeMenu::build() {
    setPolicy(USE_MIN_SIZE);
    _screenTitleFrame.setPreferedSize(Vec3(0, 20));
    _screenTitleFrame.add(&_themeMenuTitle);
    add(&_screenTitleFrame);
    _buttonsBox.add(&_themeList);
    _buttonsBox.add(&_backButton);
    add(&_buttonsBox);
    _themeList.build();
}
