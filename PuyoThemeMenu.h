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

#ifndef PUYO_THEME_MENU_H
#define PUYO_THEME_MENU_H

#include "gameui.h"
#include "PuyoCommander.h"
#include "AnimatedPuyoTheme.h"
#include "LevelThemeMenu.h"

class PuyoThemePicturePreview : public Widget, public IdleComponent {
    public:
        PuyoThemePicturePreview();

        // Implements IdleComponent
        virtual void idle(double currentTime);
        virtual IdleComponent *getIdleComponent() { return this; }

        void build();
        void themeSelected(AnimatedPuyoSetTheme * theme);

    protected:
      void draw(SDL_Surface *screen);
      SoFont *font;
      
    private:
      String label;
      AnimatedPuyoSetTheme * curTheme;
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
        void themeSelected(String themeName);

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
    void action(Widget *sender, int actionType, GameControlEvent *event);
private:
    PuyoThemePreview themePreview;
    IIM_Surface *leftArrow, *rightArrow;
    Image *prevButton, *nextButton;
    ZBox Spacer1, Spacer2;
};

class PuyoThemeMenu : public PuyoMainScreenMenu {
public:
    PuyoThemeMenu(PuyoMainScreen *mainScreen);
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
