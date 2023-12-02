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

#ifndef LEVEL_THEME_MENU_H
#define LEVEL_THEME_MENU_H

#include "gameui.h"
#include "MainScreen.h"
#include "FPCommander.h"
#include "Theme.h"

class LevelThemePicturePreview : public Widget, public IdleComponent {
    public:
        LevelThemePicturePreview();
		~LevelThemePicturePreview();
        // Implements IdleComponent
        virtual void idle(double currentTime);
        virtual IdleComponent *getIdleComponent() { return this; }
		void setSize(const Vec3 &v3);

        void build();
        void themeSelected(LevelTheme * theme);
        void draw(DrawTarget *dt);
    protected:
      void updatePicture(DrawTarget *dt);

    private:
      String label;
      std::unique_ptr<IosSurface> picture;
      std::unique_ptr<IosSurface> lilback;
      double offsetX;
      double offsetY;
      bool shouldRecache;
      bool shouldResize;
      LevelTheme * curTheme;
};

class LevelThemePreview : public VBox {
    public:
        LevelThemePreview();
        virtual ~LevelThemePreview();

        void build();
        void setSelectedTheme(std::string themeName);

    private:
        Text name;
        //Text author;
        LevelThemePicturePreview picture;
        Text description;
};

class LevelThemeSelectionBox : public HBox, public Action {
public:
    LevelThemeSelectionBox(int nbPlayers);
    virtual ~LevelThemeSelectionBox();
    void build();
    void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
private:
    int m_nbPlayers;
    LevelThemePreview themePreview;
    Image *prevButton, *nextButton;
    ZBox Spacer0, Spacer1, Spacer2, Spacer3;
};

class LevelThemeMenu : public MainScreenMenu {
public:
    LevelThemeMenu(MainScreen *mainScreen, int nbPlayers = 2);
    void build();
private:
    Frame screenTitleFrame;
    Text themeMenuTitle;
    VBox buttonsBox;
    PopMainScreenMenuAction popAction;
    Button backButton;
    LevelThemeSelectionBox themeList;
};

#endif
