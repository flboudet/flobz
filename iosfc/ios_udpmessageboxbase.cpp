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
#include <memory>
#include "ios_udpmessageboxbase.h"
#include "ios_udpmessage.h"
#include "ios_socketaddress.h"
#include "ios_time.h"

#include <sys/time.h>
#include <stdint.h>

using namespace std;
using namespace ios_fc::_private_udpmessagebox;

namespace ios_fc {

class UDPRawMessage;

/**
 * Private class for handling UDP sessions
 */
class UDPMessageBoxBase::KnownPeer {
public:
    KnownPeer(const PeerAddress &address, int receiveSerialID, UDPMessageBoxBase &owner);
    ~KnownPeer();

    /// @brief Sends a all possible elements of the queue...
    void sendQueue();

    void idle(double time_ms);
    void handleMessage(UDPMessageInterface &message, int messageSerialID);
    void handleAck(int messageSerialID);

    double getTimeMsSinceLastMessage() { return timeMsSinceLastMessage; }

    // Members

    PeerAddress address;
    UDPRawMessage *waitingForAckMessage;
    AdvancedBuffer<UDPRawMessage*> outQueue;

private:
    int receiveSerialID;
    UDPMessageBoxBase &owner;
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


UDPMessageBoxBase::KnownPeer::KnownPeer(const PeerAddress &address, int receiveSerialID, UDPMessageBoxBase &owner)
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
    for (std::list<SessionListener *>::iterator listIter = owner.m_sessionListeners.begin() ;
         listIter != owner.m_sessionListeners.end() ; ++listIter) {
        (*listIter)->onPeerConnect(address);
    }
}

UDPMessageBoxBase::KnownPeer::~KnownPeer()
{
    for (int i = 0, j = outQueue.size() ; i < j ; i++) {
        delete outQueue[i];
    }
    if (waitingForAckMessage != NULL)
        delete waitingForAckMessage;
}

void UDPMessageBoxBase::KnownPeer::sendQueue()
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


void UDPMessageBoxBase::KnownPeer::idle(double time_ms)
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
}

void UDPMessageBoxBase::KnownPeer::handleMessage(UDPMessageInterface &incomingMessage, int messageSerialID)
{
    timeMsSinceLastMessage = 0;

    // Handle message with inconsistent serial ID
    if ((messageSerialID >= 0) && (messageSerialID < receiveSerialID)) {
        // peer reset
        if ((messageSerialID <= 10) || (receiveSerialID - messageSerialID > 10)) {
            //printf("Peer reset!\n");
            owner.deletePeer(address);
            return;
        }
    }

    // We should acknowledge every reliable message
    if (incomingMessage.isReliable()) {
        owner.sendAckMessage(messageSerialID, incomingMessage.getPeerAddress());
    }

    // Drop if message has been received twice  or has come after the next one
    if (messageSerialID <= receiveSerialID) {
        // Message dropped
    }
    else {
        receiveSerialID = messageSerialID;
        // Give the message to every listener
        owner.propagateMessageToListeners(incomingMessage);
    }
}

void UDPMessageBoxBase::KnownPeer::handleAck(int messageSerialID)
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

UDPMessageBoxBase::UDPMessageBoxBase(DatagramSocket *socket)
    : defaultAddress(socket->getConnectedAddress())
    , defaultPort(socket->getConnectedPortNum())
    , socket(socket)
    , sendSerialID(0)
    , timeMsBeforeResendingReliable(500)
    , timeMsBeforeReliableTimeout(60000) // 1 minute
    , timeMsBeforePeerTimeout(60000) // 1 minute
{
}

UDPMessageBoxBase::~UDPMessageBoxBase()
{
    for (std::map<PeerAddress, KnownPeer*>::iterator iter = m_knownPeers.begin() ;
         iter != m_knownPeers.end() ; ++iter) {
        delete iter->second;
    }
}

void UDPMessageBoxBase::idle()
{
    double time_ms = getTimeMs();

    char receiveData[2048];
    Buffer<char> receiveBuffer(receiveData, 2048);

    // Known peers idle task
    for (std::map<PeerAddress, KnownPeer*>::iterator iter = m_knownPeers.begin() ;
         iter != m_knownPeers.end() ; ) {
        std::map<PeerAddress, KnownPeer*>::iterator currentIter = iter++;
        currentIter->second->idle(time_ms);
        // Handle peer timeout
        if (currentIter->second->getTimeMsSinceLastMessage() >= getTimeMsBeforePeerTimeout()) {
            //printf("Peer disconnected!\n");
            deletePeer(currentIter->first);
        }
    }

    while (socket->available()) {
        Datagram receivedDatagram = socket->receive(receiveBuffer);
        //printf("message UDP recu dans l'UDP Message Box de %s:%d !\n", (const char *)(receivedDatagram.getAddress().asString()), receivedDatagram.getPortNum());
        try {
            if (receivedDatagram.getSize() > 0) {
                auto_ptr<UDPMessageInterface> incomingMessage(createMessageFromSerialized(Buffer<char>((char *)(receivedDatagram.getMessage()), receivedDatagram.getSize()),
                                           receivedDatagram.getAddress(), receivedDatagram.getPortNum()));
                int messageSerialID = incomingMessage->getSerialID();

                // ACK message MUST be handled here, because sender address might not be the same as replier one
                if (messageSerialID < 0) {
                    for (std::map<PeerAddress, KnownPeer*>::iterator iter = m_knownPeers.begin() ;
                         iter != m_knownPeers.end() ; ++iter) {
                        iter->second->handleAck(messageSerialID);
                    }
                }
                // Else give the message to the known peer
                else {
                    KnownPeer *currentPeer = findPeer(incomingMessage->getPeerAddress());
                    if (currentPeer == NULL) {
                        currentPeer = new KnownPeer(incomingMessage->getPeerAddress(),
                                                    messageSerialID <= 0 ? 0 : messageSerialID - 1,
                                                    *this);
                        m_knownPeers[incomingMessage->getPeerAddress()] = currentPeer;
                    }
                    currentPeer->handleMessage(*incomingMessage, messageSerialID);
                }
            }
        }
        catch (Message::InvalidMessageException e) {
            receiveBuffer[2047] = 0;
            //printf("Message dropped : %s\n", (const char *)receiveBuffer);
            // Do nothing
        }
    }
}

void UDPMessageBoxBase::sendUDP(Buffer<char> buffer, int id, bool reliable, PeerAddress peerAddr, SocketAddress addr, int portNum)
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
        m_knownPeers[peerAddr] = currentPeer;
    }
    currentPeer->outQueue.add(rawMessage);
    currentPeer->sendQueue();
}

UDPMessageBoxBase::KnownPeer *UDPMessageBoxBase::findPeer(PeerAddress address)
{
    std::map<PeerAddress, KnownPeer*>::iterator iter = m_knownPeers.find(address);
    if (iter == m_knownPeers.end())
        return NULL;
    return iter->second;
}

void UDPMessageBoxBase::deletePeer(PeerAddress address)
{
    std::map<PeerAddress, KnownPeer*>::iterator iter = m_knownPeers.find(address);
    if (iter == m_knownPeers.end())
        return;
    for (std::list<SessionListener *>::iterator listIter = m_sessionListeners.begin() ;
         listIter != m_sessionListeners.end() ; ++listIter) {
        (*listIter)->onPeerDisconnect(address);
    }
    delete iter->second;
    m_knownPeers.erase(iter);
}

void UDPMessageBoxBase::addSessionListener(SessionListener *l)
{
    m_sessionListeners.push_back(l);
}

void UDPMessageBoxBase::removeSessionListener(SessionListener *l)
{
    m_sessionListeners.remove(l);
}

void UDPMessageBoxBase::bind(PeerAddress addr)
{
    UDPPeerAddressImpl *newPeerAddressImpl = dynamic_cast<UDPPeerAddressImpl *>(addr.getImpl());
    if (newPeerAddressImpl != NULL) {
        defaultAddress = newPeerAddressImpl->getAddress();
        defaultPort = newPeerAddressImpl->getPortNum();
    }
    else throw Exception("Incompatible peer address type!");
}

PeerAddress UDPMessageBoxBase::getBound() const
{
    return UDPPeerAddress(defaultAddress, defaultPort);
}

PeerAddress UDPMessageBoxBase::createPeerAddress(SocketAddress address, int portNum) const
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
