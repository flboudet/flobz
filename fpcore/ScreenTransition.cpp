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

#include "ScreenTransition.h"



ScreenTransitionWidget::ScreenTransitionWidget(Screen &fromScreen,
                                               ImageType fromScreenImageType)
{
    DrawContext *dc = GameUIDefaults::GAME_LOOP->getDrawContext();
    _fromSurface.reset(dc->getImageLibrary().createImage(fromScreenImageType, dc->getWidth(), dc->getHeight()));
    fromScreen.drawAnyway(_fromSurface.get());
}

DoomMeltScreenTransitionWidget::DoomMeltScreenTransitionWidget(Screen &fromScreen)
    : ScreenTransitionWidget(fromScreen),
      CycledComponent(.03)
{
    _melt = doom_melt_new();
    doom_melt_start(_melt, getFromSurface());
}

DoomMeltScreenTransitionWidget::~DoomMeltScreenTransitionWidget()
{
    doom_melt_delete(_melt);
}

void DoomMeltScreenTransitionWidget::cycle()
{
    if (!doom_melt_finished(_melt)) {
        doom_melt_update(_melt);
        requestDraw();
    }
    else {
        releaseFromSurface();
        parentLoop->removeIdle(this);
    }
}

void DoomMeltScreenTransitionWidget::draw(DrawTarget *dt)
{
    dt->setClipRect(NULL);
    doom_melt_display(_melt, dt);
}

