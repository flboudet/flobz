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

#include "ios_selector.h"

namespace ios_fc {

Selector::Selector()
{
    impl = factory->createSelectorImpl();
}

Selector::~Selector()
{
    delete impl;
}

void Selector::addSelectable(Selectable *s)
{
    impl->addSelectable(s);
}

void Selector::removeSelectable(Selectable *s)
{
    impl->removeSelectable(s);
}

void Selector::select()
{
    impl->selectImpl();
}

Buffer<Selectable *> Selector::getSelected()
{
    return impl->getSelected();
}

};

