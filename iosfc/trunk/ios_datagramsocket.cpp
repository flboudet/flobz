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

#include "ios_datagramsocket.h"
#include "ios_exception.h"

namespace ios_fc {

DatagramSocket::DatagramSocket(const String hostName, int localPortNum, int remotePortNum)
{
    impl = factory->createDatagramSocket();
    impl->create(localPortNum);
    sImpl = impl->getSelectableImpl();
}

DatagramSocket::DatagramSocket(int localPortNum)
{
    impl = factory->createDatagramSocket();
    impl->create(localPortNum);
    sImpl = impl->getSelectableImpl();
}

DatagramSocket::DatagramSocket()
{
    impl = factory->createDatagramSocket();
    impl->create(NODEFINEDPORT);
    sImpl = impl->getSelectableImpl();
}

DatagramSocket::DatagramSocket(DatagramSocketImpl *impl) : impl(impl)
{
    sImpl = impl->getSelectableImpl();
}
        
DatagramSocket::~DatagramSocket()
{
    delete impl;
}

void DatagramSocket::send(Datagram &sendDatagram)
{
    impl->send(sendDatagram);
}

Datagram DatagramSocket::receive(VoidBuffer buffer)
{
    return impl->receive(buffer);
}

int DatagramSocket::available() const
{
    return impl->available();
}

SelectableImpl *DatagramSocket::getSelectableImpl() const
{
    return sImpl;
}

}


