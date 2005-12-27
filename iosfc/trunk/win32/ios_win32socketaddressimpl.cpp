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

#include "ios_win32socketaddressimpl.h"
#include "ios_exception.h"

namespace ios_fc {

Win32SocketAddressImpl::Win32SocketAddressImpl(String hostName)
{
    struct hostent *ht;
    
    /* go find out about the desired host machine */
    if ((ht = gethostbyname(hostName)) == 0) {
        throw Exception("IosSocketAddress: gethostbyname error");
    }
    address = ntohl(((struct in_addr *)(ht->h_addr))->s_addr);
}

Win32SocketAddressImpl::Win32SocketAddressImpl(unsigned long address) : address(address)
{
}

bool Win32SocketAddressImpl::operator == (const SocketAddressImpl &) const
{
    return true;
}

String Win32SocketAddressImpl::asString() const
{
  String result;
  result += (int)((address & 0xFF000000) >> 24);
  result += ".";
  result += (int)((address & 0x00FF0000) >> 16);
  result += ".";
  result += (int)((address & 0x0000FF00) >> 8);
  result += ".";
  result += (int)(address & 0x000000FF);
  return result;
}

SocketAddressImpl * Win32SocketAddressFactory::createSocketAddress(String hostName)
{
    return new Win32SocketAddressImpl(hostName);
}

Win32SocketAddressFactory win32SocketAddressFactory;

SocketAddressFactory *SocketAddress::factory = &win32SocketAddressFactory;

}

