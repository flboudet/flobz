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

DatagramSocket::DatagramSocket(int localPortNum)
: impl(factory->createDatagramSocket()), sImpl(0)
{
    impl->create(localPortNum);
    sImpl = impl->getSelectableImpl();
}

DatagramSocket::DatagramSocket()
: impl(factory->createDatagramSocket()), sImpl(0)
{
    impl->create(NODEFINEDPORT);
    sImpl = impl->getSelectableImpl();
}

DatagramSocket::DatagramSocket(DatagramSocketImpl *impl)
    : impl(impl)
    , sImpl(this->impl->getSelectableImpl())
{
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

void DatagramSocket::joinGroup(SocketAddress groupAddress)
{
    impl->joinGroup(groupAddress);
}

void DatagramSocket::setMulticastInterface(SocketAddress interfaceAddress)
{
    impl->setMulticastInterface(interfaceAddress);
}

void DatagramSocket::connect(SocketAddress addr, int portNum)
{
    impl->connect(addr, portNum);
}

void DatagramSocket::disconnect()
{
    impl->disconnect();
}

bool DatagramSocket::getConnected()
{
  return impl->getConnected();
}

SocketAddress DatagramSocket::getConnectedAddress()
{
  return impl->getConnectedAddress();
}

int DatagramSocket::getConnectedPortNum()
{
  return impl->getConnectedPortNum();
}

SocketAddress DatagramSocket::getBroadcastAddress() const
{
    return impl->getBroadcastAddress();
}

SocketAddress DatagramSocket::getSocketAddress() const
{
    return impl->getSocketAddress();
}

int DatagramSocket::getSocketPortNum() const
{
    return impl->getSocketPortNum();
}

SelectableImpl *DatagramSocket::getSelectableImpl() const
{
    return sImpl;
}

}


