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

#ifndef _IOS_UDP_MESSAGE_BOX_H
#define _IOS_UDP_MESSAGE_BOX_H

#include "ios_memory.h"
#include "ios_messagebox.h"
#include "ios_datagramsocket.h"
#include "ios_socketaddress.h"
#include "ios_dirigeable.h"

namespace ios_fc {

class UDPMessageBox;
class UDPRawMessage;

class UDPMessageBox : public MessageBox, SessionManager {
public:
    UDPMessageBox(String address, int localPort, int remotePort);
    UDPMessageBox(DatagramSocket *socket);
    virtual ~UDPMessageBox() {}
    void idle();
    Message * createMessage();
    void sendUDP(Buffer<char> buffer, int id, bool reliable, PeerAddress peerAddr, SocketAddress addr, int portNum);
    SocketAddress getBroadcastAddress() const { return socket->getBroadcastAddress(); }
    SocketAddress getSocketAddress() const { return socket->getSocketAddress(); }
    DatagramSocket *getDatagramSocket() const { return socket; }
    // Session manager
    void addSessionListener(SessionListener *l);
    void removeSessionListener(SessionListener *l);
    
    // Accessors
    inline int getCyclesBeforeResendingReliable() const { return cyclesBeforeResendingReliable; }
    inline int getCyclesBeforeReliableTimeout() const { return cyclesBeforeReliableTimeout; }
    inline int getCyclesBeforePeerTimeout() const { return cyclesBeforePeerTimeout; }
private:
    struct KnownPeer;
    KnownPeer *findPeer(PeerAddress address);
    // For KnownPeers
    void warnListeners(Message &message);
    
    SocketAddress defaultAddress;
    int defaultPort;
    DatagramSocket *socket;
    int sendSerialID;
    AdvancedBuffer<KnownPeer*> knownPeers;
    // UDP Messagebox parameters
    int cyclesBeforeResendingReliable;
    int cyclesBeforeReliableTimeout;
    int cyclesBeforePeerTimeout;
    
    // Session manager
    AdvancedBuffer<SessionListener *> sessionListeners;
};

};

#endif // _IOS_UDP_MESSAGE_BOX_H
