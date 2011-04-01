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

#ifndef _IOS_UDPMESSAGEBOX_H_
#define _IOS_UDPMESSAGEBOX_H_

#include "ios_udpmessageboxbase.h"
#include "ios_udpmessage.h"

namespace ios_fc {

template <typename T>
class UDPMessageBox : public UDPMessageBoxBase {
public:
    UDPMessageBox(DatagramSocket *socket);
    virtual Message * createMessage();
protected:
    virtual ios_fc::_private_udpmessagebox::UDPMessageInterface * createMessageFromSerialized(const VoidBuffer &serialized, SocketAddress address, int port);
    virtual void sendAckMessage(int serialID, const PeerAddress &address);
private:
    UDPMessageBox (const UDPMessageBox&);
    void operator=(const UDPMessageBox&);
};

/// Implementation
template <typename T>
UDPMessageBox<T>::UDPMessageBox(DatagramSocket *socket)
    : UDPMessageBoxBase(socket)
{}

template <typename T>
Message * UDPMessageBox<T>::createMessage()
{
    ios_fc::_private_udpmessagebox::UDPMessage<T> *newMessage;
    newMessage = new ios_fc::_private_udpmessagebox::UDPMessage<T>(++sendSerialID, *this,
				defaultAddress, defaultPort);
    return newMessage;
}

template <typename T>
ios_fc::_private_udpmessagebox::UDPMessageInterface *
UDPMessageBox<T>::createMessageFromSerialized(const VoidBuffer &serialized, SocketAddress address, int port)
{
    ios_fc::_private_udpmessagebox::UDPMessage<T> *incomingMessage
        = new ios_fc::_private_udpmessagebox::UDPMessage<T>(serialized, *this,
                                                            address, port);
    return incomingMessage;
}

template <typename T>
void UDPMessageBox<T>::sendAckMessage(int serialID, const PeerAddress &address)
{
    ios_fc::_private_udpmessagebox::UDPMessage<T>
        acknowledgeMessage(-serialID, *this, address);
    acknowledgeMessage.send();
}

}

#endif
