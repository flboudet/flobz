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
#include "ios_time.h"

#include <sys/time.h>
#include <stdint.h>

namespace ios_fc {

/**
 * Private class for handling UDP sessions
 */
class UDPMessageBox::KnownPeer {
public:
    KnownPeer(const PeerAddress &address, int receiveSerialID, UDPMessageBox &owner);
    ~KnownPeer();

    /// @brief Sends a all possible elements of the queue...
    void sendQueue();

    void idle(double time_ms);
    void handleMessage(UDPMessage &message, int messageSerialID);
    void handleAck(int messageSerialID);

    // Members
    
    PeerAddress address;
    UDPRawMessage *waitingForAckMessage;
    AdvancedBuffer<UDPRawMessage*> outQueue;

private:
    int receiveSerialID;
    UDPMessageBox &owner;
    double waitingForAckTimeout;
    double waitingForAckLifespan;
    double timeMsSinceLastMessage;
    double lastIdle;

    KnownPeer(const KnownPeer&);
    KnownPeer&operator=(const KnownPeer&);
};

class UDPRawMessage {
public:
    UDPRawMessage(Buffer<char> buf, int msgid, int reliableFlag,
		  SocketAddress &address, int port, DatagramSocket &sock)
      : buffer()
      , id(msgid)
      , reliable(reliableFlag)
      , address(address)
      , port(port)
      , socket(sock)
      {
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
    : address(address)
    , waitingForAckMessage(NULL)
    , outQueue()
    , receiveSerialID(receiveSerialID)
    , owner(owner)
    , waitingForAckTimeout(0)
    , waitingForAckLifespan(0)
    , timeMsSinceLastMessage(0)
    , lastIdle(0)
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
    while ((waitingForAckMessage == NULL) && (outQueue.size() > 0))
    {
        UDPRawMessage *currentMessage = outQueue[0];
        currentMessage->send();
        outQueue.removeKeepOrder(currentMessage);
        if (currentMessage->isReliable()) {
            // Store the current Ack-awaited message.
            waitingForAckMessage  = currentMessage;
            // Time remaining before dropping.
            waitingForAckTimeout  = owner.getTimeMsBeforeResendingReliable();
            // Time we've waited for an Ack.
            waitingForAckLifespan = 0;
        }
        else {
            // We don't need this message anymore, bye man!
            delete currentMessage;
        }
    }
}


void UDPMessageBox::KnownPeer::idle(double time_ms)
{
    double elapsed_ms = time_ms - lastIdle;
    if (lastIdle == 0) {
        elapsed_ms = 0;
    }
    lastIdle = time_ms;

    // Handle reliable message reemission
    if (waitingForAckMessage != NULL)
    {
        // Time to "drop".
        if (waitingForAckTimeout  > elapsed_ms) {
            waitingForAckTimeout  -= elapsed_ms;
        }
        else {
            waitingForAckTimeout = 0;
        }
        // Time since "send".
        waitingForAckLifespan += elapsed_ms;

        if (waitingForAckLifespan >= owner.getTimeMsBeforeReliableTimeout())
        {
            printf("Reliable message dropped!\n");
            delete waitingForAckMessage;
            waitingForAckMessage = NULL;
        }
        else if (waitingForAckTimeout <= 0)
        {
            waitingForAckTimeout = owner.getTimeMsBeforeResendingReliable();
            waitingForAckMessage->send();
        }
    }
    
    // Handle peer timeout
    timeMsSinceLastMessage += elapsed_ms;
    if (timeMsSinceLastMessage >= owner.getTimeMsBeforePeerTimeout()) {
        printf("Peer disconnected!\n");
        delete this;
    }
}

void UDPMessageBox::KnownPeer::handleMessage(UDPMessage &incomingMessage, int messageSerialID)
{
    timeMsSinceLastMessage = 0;
    
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

void UDPMessageBox::KnownPeer::handleAck(int messageSerialID)
{
    if (waitingForAckMessage != NULL)
    {
        // If the incoming message is the ACK for the waitingForAckMessage, do what must be done
        if (waitingForAckMessage->getSerialID() == -messageSerialID)
        {
            timeMsSinceLastMessage = 0;
            delete waitingForAckMessage;
            waitingForAckMessage = NULL;
            sendQueue();
            return;
        }
    }
}

UDPMessageBox::UDPMessageBox(const String address,
			     int localPort, int remotePort)
    : defaultAddress(address)
    , defaultPort(remotePort)
    , socket(new DatagramSocket(localPort))
    , sendSerialID(0)
    , knownPeers()
    , timeMsBeforeResendingReliable(500)
    , timeMsBeforeReliableTimeout(60000) // 1 minute
    , timeMsBeforePeerTimeout(60000) // 1 minute
    , sessionListeners()
{
    socket->connect(defaultAddress, remotePort);
}

UDPMessageBox::UDPMessageBox(DatagramSocket *socket)
    : defaultAddress("localhost")
    , defaultPort(socket->getSocketPortNum())
    , socket(socket)
    , sendSerialID(0)
    , knownPeers()
    , timeMsBeforeResendingReliable(500)
    , timeMsBeforeReliableTimeout(60000) // 1 minute
    , timeMsBeforePeerTimeout(60000) // 1 minute
    , sessionListeners()
{
}

UDPMessageBox::~UDPMessageBox()
{
  while(knownPeers.size()>0) delete knownPeers[0];
}

void UDPMessageBox::idle()
{
    double time_ms = getTimeMs();
      
    char receiveData[2048];
    Buffer<char> receiveBuffer(receiveData, 2048);
    
    // Known peers idle task
    for (int i = 0, j = knownPeers.size() ; i < j ; i++) {
        knownPeers[i]->idle(time_ms);
    }
    
    while (socket->available()) {
        Datagram receivedDatagram = socket->receive(receiveBuffer);
        //printf("message UDP recu dans l'UDP Message Box de %s:%d !\n", (const char *)(receivedDatagram.getAddress().asString()), receivedDatagram.getPortNum());
        try {
            if (receivedDatagram.getSize() > 0) {
                UDPMessage incomingMessage(Buffer<char>((char *)(receivedDatagram.getMessage()), receivedDatagram.getSize()),
                                           *this, receivedDatagram.getAddress(), receivedDatagram.getPortNum());
                
                int messageSerialID = incomingMessage.getSerialID();
                
                // ACK message MUST be handled here, because sender address might not be the same as replier one
                if (messageSerialID < 0) {
                    for (int i = 0, j = knownPeers.size() ; i < j ; i++) {
                        knownPeers[i]->handleAck(messageSerialID);
                    }
                }
                // Else give the message to the known peer
                else {
                    KnownPeer *currentPeer = findPeer(incomingMessage.getPeerAddress());
                    if (currentPeer == NULL) {
                        currentPeer = new KnownPeer(incomingMessage.getPeerAddress(),
                                                    messageSerialID <= 0 ? 0 : messageSerialID - 1,
                                                    *this);
                    }
                    currentPeer->handleMessage(incomingMessage, messageSerialID);
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

void UDPMessageBox::sendUDP(Buffer<char> buffer, int id, bool reliable, PeerAddress peerAddr, SocketAddress addr, int portNum)
{
    UDPRawMessage *rawMessage = new UDPRawMessage(buffer, id, reliable,
						  addr, portNum, *socket);
    
    // Service messages must be sent imediately
    if (id <= 0) {
        rawMessage->send();
        delete rawMessage;
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

void UDPMessageBox::bind(PeerAddress addr)
{
    UDPMessage::UDPPeerAddressImpl *newPeerAddressImpl = dynamic_cast<UDPMessage::UDPPeerAddressImpl *>(addr.getImpl());
    if (newPeerAddressImpl != NULL) {
        defaultAddress = newPeerAddressImpl->getAddress();
        defaultPort = newPeerAddressImpl->getPortNum();
    }
    else throw Exception("Incompatible peer address type!");
}

PeerAddress UDPMessageBox::getBound() const
{
    return UDPPeerAddress(defaultAddress, defaultPort);
}

PeerAddress UDPMessageBox::createPeerAddress(SocketAddress address, int portNum) const
{
    return UDPPeerAddress(address, portNum);
}


/*UDPMessageBox::UDPMessageBox (const UDPMessageBox&m)
  : defaultAddress(m.defaultAddress), defaultPort(m.defaultPort)
{
    IOS_ERROR("NOT ALLOWED!\n");
}

void UDPMessageBox::operator=(const UDPMessageBox&)
{
    IOS_ERROR("NOT ALLOWED!\n");
}*/

}


