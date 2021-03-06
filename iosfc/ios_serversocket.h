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

#ifndef _IOSSERVERSOCKET
#define _IOSSERVERSOCKET

#include "ios_socket.h"
#include "ios_selectable.h"

namespace ios_fc {

class ServerSocketImpl {
public:
    virtual ~ServerSocketImpl() {}
    virtual void create(int portID) = 0;
    virtual SocketImpl *acceptClientImpl() = 0;
    virtual SelectableImpl *getSelectableImpl() = 0;
};

class ServerSocketFactory {
public:
    virtual ServerSocketImpl * createServerSocket() = 0;
};



class ServerSocket : public Selectable {
public:
    ServerSocket(int portID);
    virtual ~ServerSocket();
    Socket *acceptClient();
    
    class AcceptedClientFactory {
    public:
        virtual Socket *createSocket(SocketImpl *impl) = 0;
    };
    Socket *acceptClient(AcceptedClientFactory &acFactory);
    
    // Selectable implementation
    SelectableImpl *getSelectableImpl() const;
protected:
    int portID;
private:
    static ServerSocketFactory *factory;
    ServerSocketImpl *impl;
    SelectableImpl *sImpl;
};

};
#endif //_IOSSERVERSOCKET

