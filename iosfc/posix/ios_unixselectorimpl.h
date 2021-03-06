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

#ifndef _IOSUNIXSELECTORIMPL
#define _IOSUNIXSELECTORIMPL

#include "ios_selector.h"
#include <sys/types.h>
#include <sys/select.h>

namespace ios_fc {

class UnixSelectableImpl : public SelectableImpl {
public:
    virtual int getFd() = 0;
};

class UnixSelectorImpl : public SelectorImpl {
public:
    UnixSelectorImpl();
    virtual ~UnixSelectorImpl();
    virtual void addSelectable(Selectable *s);
    virtual void removeSelectable(Selectable *s);
    virtual void selectImpl();
    virtual void selectImpl(int timeout);
    virtual Buffer<Selectable *> getSelected();
private:
    void doSelect(struct timeval *timeout);
    AdvancedBuffer<Selectable *> selectableList;
    fd_set readfds;
};

class UnixSelectorFactory : public SelectorFactory {
public:
    virtual SelectorImpl * createSelectorImpl();
};

}

#endif // _IOSUNIXSELECTORIMPL
