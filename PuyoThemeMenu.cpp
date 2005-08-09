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

class PuyoThemeSelectedAction : public Action {
public:
    PuyoThemeSelectedAction(PuyoThemePreview &previewWidget, String themeName);
    void action();
private:
    PuyoThemePreview &previewWidget;
    String themeName;
};

PuyoThemeSelectedAction::PuyoThemeSelectedAction(PuyoThemePreview &previewWidget, String themeName)
    : previewWidget(previewWidget), themeName(themeName)
{
}

void PuyoThemeSelectedAction::action()
{
    previewWidget.themeSelected(themeName);
    getPuyoThemeManger()->setPreferedAnimatedPuyoSetTheme(themeName);
}


PuyoThemeSelectionBox::PuyoThemeSelectionBox(PuyoThemePreview &themePreview)
    : themePreview(themePreview)
{
}

PuyoThemeSelectionBox::~PuyoThemeSelectionBox()
{
    int size = buttonList.size();
    for (int i = 0; i < size; i++)
    {
        delete buttonList[i];
        delete actionList[i];
    }
}

void PuyoThemeSelectionBox::build()
{
    AdvancedBuffer<const char *> * themes = 
        getPuyoThemeManger()->getAnimatedPuyoSetThemeList();
    String pref = getPuyoThemeManger()->getPreferedAnimatedPuyoSetThemeName();
    int size = themes->size();
    for (int i = 0; i < size; i++)
    {
        Action * a = new PuyoThemeSelectedAction(themePreview, (*themes)[i]);
        Button * b = new Button((*themes)[i], a);
        buttonList.add(b);
        actionList.add(a);
        add(b);
        if (pref == (*themes)[i])
        {
            focus(b);
            themePreview.themeSelected(pref);
        }
    }
}

#define MARGIN (52.)
#define ONEPUYO (32.)

PuyoThemePreview::PuyoThemePreview()
{
      this->font = GameUIDefaults::FONT_TEXT;
      setPreferedSize(Vec3(SoFont_TextWidth(this->font, label), SoFont_FontHeight(this->font), 1.0)+Vec3(0.,MARGIN,0.));
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

void PuyoThemePreview::draw(SDL_Surface *screen)
{
    SoFont_PutString(font, screen, (int)(offsetX + getPosition().x), (int)(offsetY + getPosition().y), (const char*)(label), NULL);
    if (curTheme != NULL)
    {
      IIM_Rect r;
      Vec3 size = getPreferedSize();
      r.x = (Sint16)(getPosition().x+(size.x-NUMBER_OF_PUYOS*ONEPUYO+(NUMBER_OF_PUYOS-1)*ONEPUYO/4.0)/2.0);
      r.y = (Sint16)(getPosition().y+SoFont_FontHeight(this->font)+MARGIN-ONEPUYO);
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

void PuyoThemePreview::themeSelected(String themeName)
{
    label = themeName;
    curTheme = getPuyoThemeManger()->getAnimatedPuyoSetTheme(themeName);
    requestDraw();
    setPreferedSize(Vec3(SoFont_TextWidth(this->font, label), SoFont_FontHeight(this->font), 1.0)+Vec3(0.,MARGIN,0.));
    if (parent)
      parent->arrangeWidgets();
}

void PuyoThemePreview::idle(double currentTime)
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

PuyoThemeMenu::PuyoThemeMenu(PuyoRealMainScreen *mainScreen)
    : PuyoMainScreenMenu(mainScreen), popAction(mainScreen),
      backButton("Back", &popAction), themeMenuTitle("Change Theme..."),
      themePreview(), themeList(themePreview)
{
}

void PuyoThemeMenu::build() {
    themeList.build();

    add(&themeMenuTitle);
    add(&themeList);
    add(&themePreview);
    add(&backButton);
}
