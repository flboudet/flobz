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

namespace ios_fc {

class UDPRawMessage {
public:
    UDPRawMessage(Buffer<char> buf, int msgid, int reliableFlag, IosDatagramSocket &sock) : id(msgid), reliable(reliableFlag), socket(sock) {
        buffer = buf;
    }
    virtual ~UDPRawMessage() {}
    void send() {
        socket.socketSend(buffer, buffer.size());
    }
    bool isReliable() const { return reliable; }
    int getSerialID() const { return id; }
private:
    Buffer<char> buffer;
    int id;
    bool reliable;
    IosDatagramSocket &socket;
};


UDPMessageBox::UDPMessageBox(char *address, int localPort, int remotePort) : socket(address, localPort, remotePort)
{
    sendSerialID = 0;
    receiveSerialID = 0;
    waitingForAckMessage = NULL;
    cyclesBeforeResendingReliable = 10;
}

void UDPMessageBox::idle()
{
    char receiveBuffer[2048];
    int bufferSize;
    
    // Resend the waitingForAckMessage when it has reached its timeout
    if (waitingForAckMessage != NULL) {
        waitingForAckTimeout--;
        if (waitingForAckTimeout <= 0) {
            waitingForAckTimeout = cyclesBeforeResendingReliable;
            waitingForAckMessage->send();
        }
    }
    
    do {
        socket.socketReceive(&receiveBuffer, bufferSize);
        
        try {
            if (bufferSize > 0) {
                UDPMessage incomingMessage(Buffer<char>(receiveBuffer, bufferSize), *this);
                
                int messageSerialID = incomingMessage.getSerialID();
                
                // We should acknowledge every reliable message
                if (incomingMessage.isReliable()) {
                    UDPMessage acknowledgeMessage(-messageSerialID, *this);
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
                    for (int i = 0, j = listeners.getSize() ; i < j ; i++) {
                        MessageListener *currentListener = (MessageListener *)(listeners.getElementAt(i));
                        currentListener->onMessage(incomingMessage);
                    }
                }
            }
        }
        catch (UDPMessage::InvalidMessageException e) {
            printf("Message dropped\n");
            // Do nothing
        }
    } while (bufferSize != 0);
}

void UDPMessageBox::sendUDP(Buffer<char> buffer, int id, bool reliable)
{
    UDPRawMessage *rawMessage = new UDPRawMessage(buffer, id, reliable, socket);
    
    // Service messages must be sent imediately
    if (id <= 0) {
        rawMessage->send();
        return;
    }
    
    outQueue.addElement(rawMessage);
    sendQueue();
}

void UDPMessageBox::sendQueue()
{
    while ((waitingForAckMessage == NULL) && (outQueue.getSize() > 0)) {
        UDPRawMessage *currentMessage = (UDPRawMessage *)(outQueue.getElementAt(0));
        currentMessage->send();
        outQueue.removeElementAt(0);
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
    newMessage = new UDPMessage(++sendSerialID, *this);
    return newMessage;
}

};


