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

#include <stdio.h>
#include <string.h>
#include "ios_udpmessagebox.h"
#include "ios_udpmessage.h"
#include "ios_socketaddress.h"

namespace ios_fc {

class UDPRawMessage {
public:
    UDPRawMessage(Buffer<char> buf, int msgid, int reliableFlag,
		  SocketAddress &address, int port, DatagramSocket &sock)
      : id(msgid), reliable(reliableFlag),
	address(address), port(port), socket(sock) {
        buffer = buf;
    }
    virtual ~UDPRawMessage() {}
    void send() {
        Datagram datagramToSend(address, port, buffer, buffer.size());
        socket.send(datagramToSend);
    }
    bool isReliable() const { return reliable; }
    int getSerialID() const { return id; }
private:
    Buffer<char> buffer;
    int id;
    bool reliable;
    SocketAddress address;
    int port;
    DatagramSocket &socket;
};


UDPMessageBox::UDPMessageBox(const String address,
			     int localPort, int remotePort)
  : defaultAddress(address), defaultPort(remotePort)
{
    sendSerialID = 0;
    receiveSerialID = 0;
    waitingForAckMessage = NULL;
    cyclesBeforeResendingReliable = 10;
    socket = new DatagramSocket(localPort);
}

UDPMessageBox::UDPMessageBox(DatagramSocket *socket)
    : defaultAddress("localhost"), defaultPort(0), socket(socket)
{
    sendSerialID = 0;
    receiveSerialID = 0;
    waitingForAckMessage = NULL;
    cyclesBeforeResendingReliable = 10;
}

void UDPMessageBox::idle()
{
    char receiveData[2048];
    Buffer<char> receiveBuffer(receiveData, 2048);
    
    // Resend the waitingForAckMessage when it has reached its timeout
    if (waitingForAckMessage != NULL) {
        waitingForAckTimeout--;
        if (waitingForAckTimeout <= 0) {
            waitingForAckTimeout = cyclesBeforeResendingReliable;
            waitingForAckMessage->send();
        }
    }
    
    while (socket->available()) {
        Datagram receivedDatagram = socket->receive(receiveBuffer);
        try {
            if (receivedDatagram.getSize() > 0) {
                UDPMessage incomingMessage(Buffer<char>((char *)(receivedDatagram.getMessage()), receivedDatagram.getSize()), *this, receivedDatagram.getAddress(), receivedDatagram.getPortNum());
                
                int messageSerialID = incomingMessage.getSerialID();
                
                // We should acknowledge every reliable message
                if (incomingMessage.isReliable()) {
                    UDPMessage acknowledgeMessage(-messageSerialID, *this, receivedDatagram.getAddress(), receivedDatagram.getPortNum());
                    acknowledgeMessage.send();
                }
                
                if (waitingForAckMessage != NULL) {
                    // If the incoming message is the ACK for the waitingForAckMessage, do what must be done
                    if (waitingForAckMessage->getSerialID() == -messageSerialID) {
                        delete waitingForAckMessage;
                        waitingForAckMessage = NULL;
                        sendQueue();
                    }
                }
                
                // Drop if message has been received twice  or has come after the next one
                if (messageSerialID <= receiveSerialID) {
                    // Message dropped
                }
                else {
                    // Give the message to every listener
                    receiveSerialID = messageSerialID;
                    for (int i = 0, j = listeners.size() ; i < j ; i++) {
                        MessageListener *currentListener = listeners[i];
                        currentListener->onMessage(incomingMessage);
                    }
                }
            }
        }
        catch (UDPMessage::InvalidMessageException e) {
            receiveBuffer[2047] = 0;
            printf("Message dropped : %s\n", (const char *)receiveBuffer);
            // Do nothing
        }
    }
}

void UDPMessageBox::sendUDP(Buffer<char> buffer, int id, bool reliable, SocketAddress addr, int portNum)
{
    UDPRawMessage *rawMessage = new UDPRawMessage(buffer, id, reliable,
						  addr, portNum, *socket);
    
    // Service messages must be sent imediately
    if (id <= 0) {
        rawMessage->send();
        return;
    }
    
    outQueue.add(rawMessage);
    sendQueue();
}

void UDPMessageBox::sendQueue()
{
    while ((waitingForAckMessage == NULL) && (outQueue.size() > 0)) {
        UDPRawMessage *currentMessage = outQueue[0];
        currentMessage->send();
        outQueue.remove(currentMessage);
        if (currentMessage->isReliable()) {
            waitingForAckMessage = currentMessage;
            waitingForAckTimeout = cyclesBeforeResendingReliable;
        }
        else {
            delete currentMessage;
        }
    }
}

Message * UDPMessageBox::createMessage()
{
    UDPMessage *newMessage;
    newMessage = new UDPMessage(++sendSerialID, *this,
				defaultAddress, defaultPort);
    return newMessage;
}

};


