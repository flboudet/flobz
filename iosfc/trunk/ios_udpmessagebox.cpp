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

/**
 * Private class for handling UDP sessions
 */
class UDPMessageBox::KnownPeer {
public:
    KnownPeer(const PeerAddress &address, int receiveSerialID, UDPMessageBox &owner);
    ~KnownPeer();
    void sendQueue();
    void idle();
    void handleMessage(UDPMessage &message);
    PeerAddress address;
    UDPRawMessage *waitingForAckMessage;
    AdvancedBuffer<UDPRawMessage*> outQueue;
private:
    int receiveSerialID;
    UDPMessageBox &owner;
    int waitingForAckTimeout;
    int waitingForAckLifespan;
    int cycleSinceLastMessage;
};

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


UDPMessageBox::KnownPeer::KnownPeer(const PeerAddress &address, int receiveSerialID, UDPMessageBox &owner)
    : address(address), waitingForAckMessage(NULL), receiveSerialID(receiveSerialID),
    owner(owner), cycleSinceLastMessage(0)
{
    owner.knownPeers.add(this);
    for (int i = 0, j = owner.sessionListeners.size() ; i < j ; i++) {
        owner.sessionListeners[i]->onPeerConnect(address);
    }
}

UDPMessageBox::KnownPeer::~KnownPeer()
{
    for (int i = 0, j = owner.sessionListeners.size() ; i < j ; i++) {
        owner.sessionListeners[i]->onPeerDisconnect(address);
    }
    owner.knownPeers.remove(this);
    for (int i = 0, j = outQueue.size() ; i < j ; i++) {
        delete outQueue[i];
    }
    if (waitingForAckMessage != NULL)
        delete waitingForAckMessage;
}

void UDPMessageBox::KnownPeer::sendQueue()
{
    while ((waitingForAckMessage == NULL) && (outQueue.size() > 0)) {
        UDPRawMessage *currentMessage = outQueue[0];
        currentMessage->send();
        outQueue.remove(currentMessage);
        if (currentMessage->isReliable()) {
            waitingForAckMessage = currentMessage;
            waitingForAckTimeout = owner.getCyclesBeforeResendingReliable();
            waitingForAckLifespan = 0;
        }
        else {
            delete currentMessage;
        }
    }
}


void UDPMessageBox::KnownPeer::idle()
{
    // Handle reliable message reemission
    if (waitingForAckMessage != NULL) {
        waitingForAckTimeout--;
        waitingForAckLifespan++;
        if (waitingForAckLifespan >= owner.getCyclesBeforeReliableTimeout()) {
            printf("Reliable message dropped!\n");
            delete waitingForAckMessage;
            waitingForAckMessage = NULL;
        }
        else if (waitingForAckTimeout <= 0) {
            waitingForAckTimeout = owner.getCyclesBeforeResendingReliable();
            waitingForAckMessage->send();
        }
    }
    
    // Handle peer timeout
    cycleSinceLastMessage++;
    if (cycleSinceLastMessage >= owner.getCyclesBeforePeerTimeout()) {
        printf("Peer disconnected!\n");
        delete this;
    }
}

void UDPMessageBox::KnownPeer::handleMessage(UDPMessage &incomingMessage)
{
    int messageSerialID = incomingMessage.getSerialID();
    
    cycleSinceLastMessage = 0;
    
    if (waitingForAckMessage != NULL) {
        // If the incoming message is the ACK for the waitingForAckMessage, do what must be done
        if (waitingForAckMessage->getSerialID() == -messageSerialID) {
            delete waitingForAckMessage;
            waitingForAckMessage = NULL;
            sendQueue();
            return;
        }
    }
    
    // Handle message with inconsistent serial ID
    if ((messageSerialID >= 0) && (messageSerialID < receiveSerialID)) {
        // peer reset
        if ((messageSerialID <= 10) || (receiveSerialID - messageSerialID > 10)) {
            printf("Peer reset!\n");
            delete(this);
            return;
        }
    }
    
    // We should acknowledge every reliable message
    if (incomingMessage.isReliable()) {
        UDPMessage acknowledgeMessage(-messageSerialID, owner, incomingMessage.getPeerAddress());
        acknowledgeMessage.send();
    }
    
    
    
    // Drop if message has been received twice  or has come after the next one
    if (messageSerialID <= receiveSerialID) {
        // Message dropped
    }
    else {
        receiveSerialID = messageSerialID;
        // Give the message to every listener
        owner.warnListeners(incomingMessage);
    }
}

UDPMessageBox::UDPMessageBox(const String address,
			     int localPort, int remotePort)
  : defaultAddress(address), defaultPort(remotePort)
{
    sendSerialID = 0;
    cyclesBeforeResendingReliable = 10;
    cyclesBeforeReliableTimeout = 2000;
    cyclesBeforePeerTimeout = 2000;
    socket = new DatagramSocket(localPort);
}

UDPMessageBox::UDPMessageBox(DatagramSocket *socket)
    : defaultAddress("localhost"), defaultPort(0), socket(socket)
{
    sendSerialID = 0;
    cyclesBeforeResendingReliable = 10;
    cyclesBeforeReliableTimeout = 2000;
    cyclesBeforePeerTimeout = 2000;
}

void UDPMessageBox::idle()
{
    char receiveData[2048];
    Buffer<char> receiveBuffer(receiveData, 2048);
    
    // Known peers idle task
    for (int i = 0, j = knownPeers.size() ; i < j ; i++) {
        KnownPeer *currentPeer = knownPeers[i];
        currentPeer->idle();
    }
    
    while (socket->available()) {
        Datagram receivedDatagram = socket->receive(receiveBuffer);
        try {
            if (receivedDatagram.getSize() > 0) {
                UDPMessage incomingMessage(Buffer<char>((char *)(receivedDatagram.getMessage()), receivedDatagram.getSize()), *this, receivedDatagram.getAddress(), receivedDatagram.getPortNum());
                
                int messageSerialID = incomingMessage.getSerialID();
                KnownPeer *currentPeer = findPeer(incomingMessage.getPeerAddress());
                if (currentPeer == NULL) {
                    currentPeer = new KnownPeer(incomingMessage.getPeerAddress(),
                                                messageSerialID <= 0 ? 0 : messageSerialID - 1,
                                                *this);
                }
                currentPeer->handleMessage(incomingMessage);
            }
        }
        catch (UDPMessage::InvalidMessageException e) {
            receiveBuffer[2047] = 0;
            printf("Message dropped : %s\n", (const char *)receiveBuffer);
            // Do nothing
        }
    }
}

void UDPMessageBox::sendUDP(Buffer<char> buffer, int id, bool reliable, PeerAddress peerAddr, SocketAddress addr, int portNum)
{
    UDPRawMessage *rawMessage = new UDPRawMessage(buffer, id, reliable,
						  addr, portNum, *socket);
    
    // Service messages must be sent imediately
    if (id <= 0) {
        rawMessage->send();
        return;
    }
    
    KnownPeer *currentPeer = findPeer(peerAddr);
    if (currentPeer == NULL) {
        currentPeer = new KnownPeer(peerAddr, 0, *this);
    }
    currentPeer->outQueue.add(rawMessage);
    currentPeer->sendQueue();
}

Message * UDPMessageBox::createMessage()
{
    UDPMessage *newMessage;
    newMessage = new UDPMessage(++sendSerialID, *this,
				defaultAddress, defaultPort);
    return newMessage;
}

UDPMessageBox::KnownPeer *UDPMessageBox::findPeer(PeerAddress address)
{
    for (int i = 0, j = knownPeers.size() ; i < j ; i++) {
        KnownPeer *currentPeer = knownPeers[i];
        if (currentPeer->address == address) {
            return currentPeer;
        }
    }
    return NULL;
}

void UDPMessageBox::warnListeners(Message &message)
{
    for (int i = 0, j = listeners.size() ; i < j ; i++) {
        MessageListener *currentListener = listeners[i];
        currentListener->onMessage(message);
    }
}

void UDPMessageBox::addSessionListener(SessionListener *l)
{
    sessionListeners.add(l);
}

void UDPMessageBox::removeSessionListener(SessionListener *l)
{
    sessionListeners.remove(l);
}

};


