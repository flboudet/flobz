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

#ifndef _FLOBOSCREENTRANSITION
#define _FLOBOSCREENTRANSITION

#include <memory>
#include "gameui.h"
#include "DoomMelt.h"

using namespace gameui;

/**
 * All ScreenTransitionWidgets have to inherit this class
 */
class ScreenTransitionWidget : public Widget
{
public:
    ScreenTransitionWidget(Screen &fromScreen);
    virtual ~ScreenTransitionWidget() {}
protected:
    IosSurface *getFromSurface() const { return m_fromSurface.get(); }
    void releaseFromSurface() { m_fromSurface.reset(NULL); }
private:
    std::auto_ptr<IosSurface> m_fromSurface;
};

/**
 * Default implementation (doom melt effect)
 */
class DoomMeltScreenTransitionWidget : public ScreenTransitionWidget, public CycledComponent {
public:
    DoomMeltScreenTransitionWidget(Screen &fromScreen);
    virtual ~DoomMeltScreenTransitionWidget();
    void cycle();
    void draw(DrawTarget *dt);
    IdleComponent *getIdleComponent() { return this; }
private:
    DoomMelt *melt;
};

#endif // _FLOBOSCREENTRANSITION
