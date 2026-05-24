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

#ifndef _THEME_MENU_H
#define _THEME_MENU_H

#include "gameui.h"
#include "FPCommander.h"
#include "Theme.h"
#include "LevelThemeMenu.h"

class FloboThemePicturePreview : public Widget, public IdleComponent {
    public:
        FloboThemePicturePreview();

        // Implements IdleComponent
        virtual void idle(double currentTime);
        virtual IdleComponent *getIdleComponent() { return this; }

        void build();
        void themeSelected(FloboSetTheme * theme);

    protected:
      void draw(DrawTarget *dt);

    private:
      std::string _label;
      FloboSetTheme * _curTheme;
      double _offsetX;
      double _offsetY;
      int _eyes[NUMBER_OF_FLOBOS];
      double _lastTime;
};

class FloboThemePreview : public VBox {
    public:
        FloboThemePreview();
        virtual ~FloboThemePreview();

        void build();
        void themeSelected(const std::string &themeName);

    private:
        Text _name;
        Text _author;
        FloboThemePicturePreview _picture;
        Text _description;
};

class FloboThemeSelectionBox : public HBox, public Action {
public:
    FloboThemeSelectionBox();
    virtual ~FloboThemeSelectionBox();
    void build();
    void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
private:
    FloboThemePreview _themePreview;
    Image *_prevButton, *_nextButton;
    ZBox _Spacer0, _Spacer1, _Spacer2, _Spacer3;
};

class ThemeMenu : public MainScreenMenu {
public:
    ThemeMenu(MainScreen *mainScreen);
    void build();
private:
    Frame _screenTitleFrame;
    Text _themeMenuTitle;
    VBox _buttonsBox;
    PopMainScreenMenuAction _popAction;
    Button _backButton;
    FloboThemeSelectionBox _themeList;
};

#endif // _THEME_MENU_H
