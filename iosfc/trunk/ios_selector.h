/* Ultimate Othello 1678
 * Copyright (C) 2002  Florent Boudet <flobo@ifrance.com>
 * iOS Software <http://ios.free.fr>
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

#ifndef _IOSSTREAMSELECTOR
#define _IOSSTREAMSELECTOR

#include "ios_memory.h"
#include "ios_selectable.h"

namespace ios_fc {

class SelectorImpl {
public:
    virtual ~SelectorImpl() {}
    virtual void addSelectable(Selectable *s) = 0;
    virtual void removeSelectable(Selectable *s) = 0;
    virtual void selectImpl() = 0;
    virtual void selectImpl(int timeout) = 0;
    virtual Buffer<Selectable *> getSelected() = 0;
};

class SelectorFactory {
public:
    virtual SelectorImpl * createSelectorImpl() = 0;
};

class Selector {
public:
    Selector();
    ~Selector();
    void addSelectable(Selectable *s);
    void removeSelectable(Selectable *s);
    void select();
    void select(int timeout);
    Buffer<Selectable *> getSelected();
private:
    static SelectorFactory *factory;
    SelectorImpl *impl;
};

};

#endif // _IOSSTREAMSELECTOR

