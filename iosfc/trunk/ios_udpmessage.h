#ifndef _IOS_UDP_MESSAGE_H
#define _IOS_UDP_MESSAGE_H

#include "ios_standardmessage.h"
#include "ios_udpmessagebox.h"
#include "ios_dirigeable.h"

namespace ios_fc {

    class UDPMessage : public StandardMessage, public Dirigeable {
    public:
        UDPMessage(int serialID, UDPMessageBox &owner, SocketAddress address, int port);
        UDPMessage(int serialID, UDPMessageBox &owner, const PeerAddress &address);
        UDPMessage(const Buffer<char> serialized, UDPMessageBox &owner, SocketAddress address, int port) throw(InvalidMessageException);
        virtual ~UDPMessage();
        void sendBuffer(Buffer<char> out) const;
        // Dirigeable implementation
        PeerAddress getPeerAddress();
        PeerAddress getBroadcastAddress();
        void setPeerAddress(PeerAddress);
        void addPeerAddress(const String key, const PeerAddress &value);
        PeerAddress getPeerAddress(const String key);
    private:
        class UDPPeerAddressImpl;
        UDPMessageBox &owner;
        UDPPeerAddressImpl *peerAddressImpl;
        PeerAddress peerAddress;
    };
    
};

#endif
