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

class PuyoThemePreview : public Widget, public IdleComponent {
    public:
        PuyoThemePreview();
        void themeSelected(String themeName);

      // Implements IdleComponent
      virtual void idle(double currentTime);
      virtual IdleComponent *getIdleComponent() { return this; }

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

class PuyoThemeSelectionBox : public HBox {
public:
    PuyoThemeSelectionBox(PuyoThemePreview &themePreview);
    virtual ~PuyoThemeSelectionBox();
    void build();
private:
    PuyoThemePreview &themePreview;
    AdvancedBuffer<Button *> buttonList;
    AdvancedBuffer<Action *> actionList;
};

class PuyoThemeMenu : public PuyoMainScreenMenu {
public:
    PuyoThemeMenu(PuyoRealMainScreen *mainScreen);
    void build();
private:
    Text themeMenuTitle;
    PuyoPopMenuAction popAction;
    Button backButton;
    PuyoThemePreview themePreview;
    PuyoThemeSelectionBox themeList;
};

#endif
