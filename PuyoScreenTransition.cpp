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

#include "PuyoScreenTransition.h"



PuyoScreenTransitionWidget::PuyoScreenTransitionWidget(Screen &fromScreen, Action *transitionFinishedAction)
    : transitionFinishedAction(transitionFinishedAction), CycledComponent(0.02)
{
    Uint32 rmask, gmask, bmask, amask;
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
           rmask = 0xff000000;
           gmask = 0x00ff0000;
           bmask = 0x0000ff00;
           amask = 0x000000ff;
    #else
           rmask = 0x000000ff;
           gmask = 0x0000ff00;
           bmask = 0x00ff0000;
           amask = 0xff000000;
    #endif
    melt = doom_melt_new();
    SDL_Surface *tmp = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                            640, 480, 32,
                                            rmask, gmask,
                                            bmask, amask);
    fromSurface = IIM_RegisterImg(SDL_DisplayFormat(tmp), false);
    SDL_FreeSurface(tmp);
    //SDL_FillRect(fromSurface->surf, NULL, 0xFFFFFFFF);
    fromScreen.drawAnyway(fromSurface->surf);
    doom_melt_start(melt, fromSurface);
}

PuyoScreenTransitionWidget::~PuyoScreenTransitionWidget()
{
    doom_melt_delete(melt);
    IIM_Free(fromSurface);
}

void PuyoScreenTransitionWidget::cycle()
{
    if (!doom_melt_finished(melt)) {
        doom_melt_update(melt);
        requestDraw();
    }
    //transitionFinishedAction.action();
}

void PuyoScreenTransitionWidget::draw(SDL_Surface *screen)
{
    //toScreen.drawAnyway(screen);
    doom_melt_display(melt, screen);
}

