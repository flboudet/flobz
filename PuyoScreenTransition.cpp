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
    : CycledComponent(.03), transitionFinishedAction(transitionFinishedAction)
{
    melt = doom_melt_new();
    DrawContext *dc = GameUIDefaults::GAME_LOOP->getDrawContext();
    fromSurface = dc->getIIMLibrary().create_DisplayFormat(dc->getWidth(), dc->getHeight());
    fromScreen.drawAnyway(fromSurface);
    doom_melt_start(melt, fromSurface);
}

PuyoScreenTransitionWidget::~PuyoScreenTransitionWidget()
{
    doom_melt_delete(melt);
    delete fromSurface;
}

void PuyoScreenTransitionWidget::cycle()
{
    if (!doom_melt_finished(melt)) {
        doom_melt_update(melt);
        requestDraw();
    }
    //transitionFinishedAction.action();
}

void PuyoScreenTransitionWidget::draw(DrawTarget *dt)
{
    //toScreen.drawAnyway(screen);
    dt->setClipRect(NULL);
    doom_melt_display(melt, dt);
}

