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
 
#ifndef _IOSUNIXSOCKETADDRESSIMPL
#define _IOSUNIXSOCKETADDRESSIMPL

#include <netdb.h>
#include <netinet/in.h>
#include "ios_memory.h"
#include "ios_socketaddress.h"

namespace ios_fc {

    class UnixSocketAddressImpl : public SocketAddressImpl {
    public:
        UnixSocketAddressImpl(String hostName);
        UnixSocketAddressImpl(in_addr_t address);
        in_addr_t getAddress() const { return address; }
    private:
        in_addr_t address;
    };

    class UnixSocketAddressFactory : public SocketAddressFactory {
    public:
        SocketAddressImpl * createSocketAddress(String hostName);
    };

}

#endif // _IOSUNIXSOCKETADDRESSIMPL

