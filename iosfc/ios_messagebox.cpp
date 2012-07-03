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

#include "ios_messagebox.h"

namespace ios_fc {

void MessageBox::addListener(MessageListener *newListener) {
    listeners.push_back(newListener);
}

void MessageBox::removeListener(MessageListener *listener) {
    for (ListOfListeners::iterator iter = listeners.begin() ;
         iter != listeners.end() ; ++iter) {
        if (*iter == listener) {
            listeners.erase(iter);
            iter = listeners.begin();
        }
    }
}

}

