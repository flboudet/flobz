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

#include "ios_unixsocketaddressimpl.h"
#include "ios_exception.h"

namespace ios_fc {

UnixSocketAddressImpl::UnixSocketAddressImpl(String hostName)
{
    struct hostent *ht;
    
    /* go find out about the desired host machine */
    if ((ht = gethostbyname(hostName)) == 0) {
        throw Exception("IosSocketAddress: gethostbyname error");
    }
    address = ((struct in_addr *)(ht->h_addr))->s_addr;
}

UnixSocketAddressImpl::UnixSocketAddressImpl(in_addr_t address) : address(address)
{
}

bool UnixSocketAddressImpl::operator == (const SocketAddressImpl &a) const
{
    const UnixSocketAddressImpl &comp = dynamic_cast<const UnixSocketAddressImpl &>(a);
    return (address == comp.address);
}

SocketAddressImpl * UnixSocketAddressFactory::createSocketAddress(String hostName)
{
    return new UnixSocketAddressImpl(hostName);
}

UnixSocketAddressFactory unixSocketAddressFactory;

SocketAddressFactory *SocketAddress::factory = &unixSocketAddressFactory;

}

