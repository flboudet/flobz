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

#include "ios_unixselectorimpl.h"

namespace ios_fc {

UnixSelectorImpl::UnixSelectorImpl()
{
    FD_ZERO(&readfds);
}

UnixSelectorImpl::~UnixSelectorImpl()
{
}

void UnixSelectorImpl::addSelectable(Selectable *s)
{
    UnixSelectableImpl *impl = dynamic_cast<UnixSelectableImpl *>(s->getSelectableImpl());
    if (impl == NULL)
        throw Exception("Selectable is not compatible with selector implementation");
    selectableList.add(s);
}

void UnixSelectorImpl::removeSelectable(Selectable *s)
{
    UnixSelectableImpl *impl = dynamic_cast<UnixSelectableImpl *>(s->getSelectableImpl());
    if (impl == NULL)
        throw Exception("Selectable is not compatible with selector implementation");
    selectableList.remove(s);
}

void UnixSelectorImpl::selectImpl()
{
    FD_ZERO(&readfds);
    for (int i = 0, j = selectableList.size() ; i < j ; i++) {
        UnixSelectableImpl *impl = static_cast<UnixSelectableImpl *>(selectableList[i]->getSelectableImpl());
        FD_SET(impl->getFd(), &readfds);
    }
    if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) == -1)
        throw Exception("IosSystemStreamSelect error");
}

Buffer<Selectable *> UnixSelectorImpl::getSelected()
{
    AdvancedBuffer<Selectable *> result;
    
    for (int i = 0, j = selectableList.size() ; i < j ; i++) {
        Selectable *currentSelectable = selectableList[i];
        UnixSelectableImpl *currentImpl = static_cast<UnixSelectableImpl *>(currentSelectable->getSelectableImpl());
        if (FD_ISSET(currentImpl->getFd(), &readfds))
            result.add(currentSelectable);
    }
    return result;
}

};