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

#ifndef _THEME_MENU_H
#define _THEME_MENU_H

#include "gameui.h"
#include "PuyoCommander.h"
#include "Theme.h"
#include "LevelThemeMenu.h"

class PuyoThemePicturePreview : public Widget, public IdleComponent {
    public:
        PuyoThemePicturePreview();

        // Implements IdleComponent
        virtual void idle(double currentTime);
        virtual IdleComponent *getIdleComponent() { return this; }

        void build();
        void themeSelected(PuyoSetTheme * theme);

    protected:
      void draw(DrawTarget *dt);

    private:
      String label;
      PuyoSetTheme * curTheme;
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
        void themeSelected(const std::string &themeName);

    private:
        Text name;
        Text author;
        PuyoThemePicturePreview picture;
        Text description;
};

class PuyoThemeSelectionBox : public HBox, public Action {
public:
    PuyoThemeSelectionBox();
    virtual ~PuyoThemeSelectionBox();
    void build();
    void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
private:
    PuyoThemePreview themePreview;
    Image *prevButton, *nextButton;
    ZBox Spacer0, Spacer1, Spacer2, Spacer3;
};

class ThemeMenu : public MainScreenMenu {
public:
    ThemeMenu(MainScreen *mainScreen);
    void build();
private:
    Frame screenTitleFrame;
    Text themeMenuTitle;
    VBox buttonsBox;
    PuyoPopMenuAction popAction;
    Button backButton;
    PuyoThemeSelectionBox themeList;
};

#endif
