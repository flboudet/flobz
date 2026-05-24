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

#include "LevelThemeMenu.h"
#include "FPStrings.h"
#include "Theme.h"

using namespace event_manager;

LevelThemeSelectionBox::LevelThemeSelectionBox(int nbPlayers)
    : _nbPlayers(nbPlayers),
      _themePreview(), _Spacer0(), _Spacer1(), _Spacer2(), _Spacer3()
{
    _prevButton = new Image(theCommander->getLeftArrow());
    _nextButton = new Image(theCommander->getRightArrow());

    _Spacer0.setPreferedSize(Vec3(15.0f, 0.0f));
    _Spacer1.setPreferedSize(Vec3(0.0f, 0.0f));
    _Spacer2.setPreferedSize(Vec3(0.0f, 0.0f));
    _Spacer3.setPreferedSize(Vec3(15.0f, 0.0f));

    setPolicy(USE_MAX_SIZE_NO_MARGIN);
}

LevelThemeSelectionBox::~LevelThemeSelectionBox()
{
    delete _prevButton;
    delete _nextButton;
}

void LevelThemeSelectionBox::build()
{
    _themePreview.build();
    std::vector<std::string> themes = theCommander->getLevelThemeList(_nbPlayers);
    std::string pref = theCommander->getPreferedLevelThemeName(_nbPlayers);
    int size = themes.size();
    bool found = false;
    for (std::vector<std::string>::const_iterator iter
             = themes.begin() ; iter != themes.end() ; iter++) {
        if (pref == *iter)
        {
            _themePreview.setSelectedTheme(pref);
            found = true;
        }
    }

    add(&_Spacer0);

    _prevButton->setFocusable(size > 1);
    _prevButton->setOnAction(this);
    _prevButton->setInvertedFocus(true);
    add(_prevButton);

    add(&_Spacer1);
    if (found == false && size > 0)
    {
        _themePreview.setSelectedTheme(themes[0]);
    }
    add(&_themePreview);
    add(&_Spacer2);

    _nextButton->setFocusable(size > 1);
    _nextButton->setOnAction(this);
    _nextButton->setInvertedFocus(true);
    add(_nextButton);

    add(&_Spacer3);
}

void LevelThemeSelectionBox::action(Widget *sender, int actionType, GameControlEvent *event)
{
    if (!event->isUp) return;
    const std::vector<std::string> &themes = theCommander->getLevelThemeList();
    std::string pref = theCommander->getPreferedLevelThemeName();
    int size = themes.size();
    if (size <= 0) return;

    int currentTheme = 0;
    // get the selected theme id or zero if the prefered theme if not there
    for (std::vector<std::string>::const_iterator iter
             = themes.begin() ; iter != themes.end() ; iter++, currentTheme++) {
        if (pref == *iter) {
            break;
        }
    }
    if (sender == _prevButton) {
        (currentTheme <= 0) ? currentTheme = size - 1 : currentTheme--;
        _themePreview.setSelectedTheme(themes[currentTheme]);
        theCommander->setPreferedLevelThemeName(themes[currentTheme].c_str());
    }
    else if (sender == _nextButton) {
        currentTheme = (currentTheme+1)%size;
        _themePreview.setSelectedTheme(themes[currentTheme]);
        theCommander->setPreferedLevelThemeName(themes[currentTheme].c_str());
    }
}

/*****************************************************************************/

#define ONELEVELX (128.)
#define ONELEVELY (96.)

LevelThemePicturePreview::LevelThemePicturePreview()
{
      _offsetX = _offsetY = 0.;
      _shouldRecache = true;
      _shouldResize = true;
      _curTheme = NULL;
      setPreferedSize(Vec3(ONELEVELX, ONELEVELY, 1.0));
}

LevelThemePicturePreview::~LevelThemePicturePreview()
{
}

void LevelThemePicturePreview::draw(DrawTarget *dt)
{
	updatePicture(dt);
    if (_lilback.get() != NULL)
    {
      IosRect r;
      Vec3 size = getSize();
      Vec3 pos = getPosition();
      r.x = (int16_t)(pos.x+_offsetX);
      r.y = (int16_t)(pos.y+_offsetY);
      r.w = (int16_t)(size.x);
      r.h = (int16_t)(size.y);
      dt->draw(_lilback.get(), NULL, &r);
    }
}

void LevelThemePicturePreview::updatePicture(DrawTarget *dt)
{
    if ((_curTheme != NULL) && (_shouldRecache == true)) {
        ImageLibrary &iimLib = GameUIDefaults::GAME_LOOP->getDrawContext()->getImageLibrary();
        IosSurface *background = _curTheme->getBackground();
        _picture.reset(iimLib.createImage(IMAGE_RGB, background->w, background->h));
        // Draw background
        _picture->draw(background, NULL, NULL);
        // Draw Grids
        IosRect r;
        IosSurface *grid = _curTheme->getGrid();
        if (grid != NULL) {
            r.x = 21;
            r.y = -1;
            r.w = grid->w;
            r.h = grid->h;
            _picture->draw(grid, NULL, &r);
            r.x = 407;
            r.y = -1;
            _picture->draw(grid, NULL, &r);
        }
        // Speed meter
        IosSurface *speedFront = _curTheme->getSpeedMeter(true);
        IosSurface *speedBack  = _curTheme->getSpeedMeter(false);
        r.x = _curTheme->getSpeedMeterX() - speedBack->w / 2;
        r.y = _curTheme->getSpeedMeterY() - speedBack->h;
        r.w = speedBack->w;
        r.h = speedBack->h;
        IosRect r2;
        _picture->draw(speedBack, NULL, &r);
        r2.x = 0;
        r2.y = speedFront->h/2;
        r2.w = speedFront->w;
        r2.h = speedFront->h/2;
        r.y += speedFront->h/2;
        _picture->draw(speedFront, &r2, &r);
        // Set status indicators
        _shouldRecache = false;
        _shouldResize = true;
    }
    if ((_shouldResize) && (_picture.get() != NULL)) {
        // Scale down
        Vec3 s=getSize();
        _lilback.reset(_picture->resizeAlpha((int)s.x,(int)s.y));
        _shouldResize = false;
    }
}

void LevelThemePicturePreview::themeSelected(LevelTheme * theme)
{
    if (theme != NULL)
    {
      _curTheme = theme;
      _shouldRecache = true;
    }
}

void LevelThemePicturePreview::setSize(const Vec3 &v3)
{
	Vec3 s=v3;
	if ((s.x/s.y) < (ONELEVELX/ONELEVELY))
	{
		s.y=s.x*ONELEVELY/ONELEVELX;
		_offsetX=0.0; _offsetY=(v3.y-s.y)/2.0;
	}
	else
	{
		s.x=s.y*ONELEVELX/ONELEVELY;
		_offsetY=0.0; _offsetX=(v3.x-s.x)/2.0;
	}
	if ((getSize().x != s.x) || (getSize().y != s.y))
	{
    	_shouldResize = true;
		Widget::setSize(s);
	}
}

void LevelThemePicturePreview::idle(double currentTime) { }


/*****************************************************************************/

LevelThemePreview::LevelThemePreview() {}

void LevelThemePreview::build() {
    _name.setFont(GameUIDefaults::FONT_TEXT);
    //author.setFont(GameUIDefaults::FONT_SMALL_INFO);
    _description.setFont(GameUIDefaults::FONT_SMALL_INFO);
    add(&_name);
    //add(&author);
    add(&_picture);
    add(&_description);
}

LevelThemePreview::~LevelThemePreview() {}

void LevelThemePreview::setSelectedTheme(std::string themeName)
{
#define _ComputeVZoneSize(A,B) Vec3(A.x>B.x?A.x:B.x,A.y+B.y+GameUIDefaults::SPACING,1.0)
    LevelTheme * curTheme = theCommander->getLevelTheme(themeName.c_str());
    if (curTheme->getAuthor() == "iOS-Software")
        _name.setValue(curTheme->getLocalizedName().c_str());
    else
        _name.setValue((themeName+" ("+curTheme->getAuthor()+")").c_str());
    //author.setValue(curTheme->getAuthor());
    _description.setValue(curTheme->getComments().c_str());
    Vec3 one=_ComputeVZoneSize(_name.getPreferedSize(),_description.getPreferedSize());
    setPreferedSize(_ComputeVZoneSize(one,_picture.getPreferedSize()));
    if (parent)
      parent->arrangeWidgets();
    _picture.themeSelected(curTheme);
}

/*****************************************************************************/

LevelThemeMenu::LevelThemeMenu(MainScreen *mainScreen, int nbPlayers)
    : MainScreenMenu(mainScreen),
      _screenTitleFrame(theCommander->getSeparatorFramePicture()),
      _themeMenuTitle(theCommander->getLocalizedString("Level theme")), _popAction(mainScreen),
      _backButton(theCommander->getLocalizedString("Back"), &_popAction),
      _themeList(nbPlayers)
{
}

void LevelThemeMenu::build() {
    setPolicy(USE_MIN_SIZE);
    _screenTitleFrame.setPreferedSize(Vec3(0, 20));
    _screenTitleFrame.add(&_themeMenuTitle);
    add(&_screenTitleFrame);
    _buttonsBox.add(&_themeList);
    _buttonsBox.add(&_backButton);
    add(&_buttonsBox);
    _themeList.build();
}


