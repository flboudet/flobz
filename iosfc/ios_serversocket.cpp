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
 
#include "ios_serversocket.h"

namespace ios_fc {

class DefaultServerSocketImpl : public ServerSocketImpl {
public:
    void create(int portID) {}
    SocketImpl *acceptClientImpl() { throwError(); }
    SelectableImpl *getSelectableImpl() { return NULL; }
private:
    void throwError() { throw Exception("Invalid socket, no socket backend set"); }
};

class DefaultServerSocketFactory : public ServerSocketFactory {
public:
    ServerSocketImpl * createServerSocket() {
        return new DefaultServerSocketImpl();
    }
};

DefaultServerSocketFactory defaultServerSocketFactory;

#ifdef DISABLED
ServerSocketFactory *ServerSocket::factory = &defaultServerSocketFactory;
#endif

ServerSocket::ServerSocket(int portID) : portID(portID)
{
    impl = factory->createServerSocket();
    impl->create(portID);
    sImpl = impl->getSelectableImpl();
}

ServerSocket::~ServerSocket()
{
    delete impl;
}

Socket *ServerSocket::acceptClient() {
    return new Socket(impl->acceptClientImpl());
}

Socket *ServerSocket::acceptClient(AcceptedClientFactory &acFactory)
{
    return acFactory.createSocket(impl->acceptClientImpl());
}

SelectableImpl *ServerSocket::getSelectableImpl() const
{
    return sImpl;
}

};

