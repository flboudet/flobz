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

#include "ios_socket.h"
#include "ios_exception.h"

namespace ios_fc {

class DefaultSocket : public SocketImpl {
public:
    void create(const String hostName, int portID) {}
    InputStream *getInputStream() { throwError(); return NULL; }
    OutputStream *getOutputStream() { throwError(); return NULL; }
    SelectableImpl *getSelectableImpl() { return NULL; }
private:
    void throwError() const { throw Exception("Invalid socket, no socket backend set"); }
};

class DefaultSocketFactory : public SocketFactory {
public:
    SocketImpl * createSocket() {
        return new DefaultSocket();
    }
};

DefaultSocketFactory defaultSocketFactory;

#ifdef DISABLED
SocketFactory *Socket::factory = &defaultSocketFactory;
#endif

Socket::Socket(const String hostName, int portID)
{
    impl = factory->createSocket();
    impl->create(hostName, portID);
    sImpl = impl->getSelectableImpl();
}

Socket::Socket(SocketImpl *impl) : impl(impl)
{
    sImpl = impl->getSelectableImpl();
}

Socket::~Socket()
{
    delete impl;
}

InputStream *Socket::getInputStream() const
{
    return impl->getInputStream();
}

OutputStream *Socket::getOutputStream() const
{
    return impl->getOutputStream();
}

SelectableImpl *Socket::getSelectableImpl() const
{
    return sImpl;
}

}
