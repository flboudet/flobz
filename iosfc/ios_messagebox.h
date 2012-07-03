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


#ifndef _IOS_MESSAGE_BOX_H
#define _IOS_MESSAGE_BOX_H

#include <vector>
#include "ios_vector.h"
#include "ios_message.h"

namespace ios_fc {

class MessageListener {
public:
    virtual void onMessage(Message &) = 0;
    virtual ~MessageListener() {};
};

class MessageBox {
public:
    MessageBox() {}
    
    virtual ~MessageBox() {}
    virtual void idle() = 0;
    void addListener(MessageListener *);
    void removeListener(MessageListener *);
    virtual Message * createMessage() = 0;
protected:
    typedef std::vector<MessageListener *> ListOfListeners;
    ListOfListeners listeners;
};

}

#endif // _IOS_MESSAGE_BOX_H
