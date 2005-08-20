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
        // Dirigeable implementation
        PeerAddress getPeerAddress();
        PeerAddress getBroadcastAddress();
        void setPeerAddress(PeerAddress);
        void addPeerAddress(const String key, const PeerAddress &value);
        PeerAddress getPeerAddress(const String key);
        class UDPPeerAddressImpl;
    private:
        void sendBuffer(Buffer<char> out) const;
        UDPMessageBox &owner;
        UDPPeerAddressImpl *peerAddressImpl;
        PeerAddress peerAddress;

        UDPMessage(const UDPMessage&);
        UDPMessage&operator=(const UDPMessage&);
    };
    
    class UDPMessage::UDPPeerAddressImpl : public PeerAddressImpl {
    public:
        UDPPeerAddressImpl(SocketAddress address, int port)
        : address(address), port(port) {}
        SocketAddress getAddress() const { return address; }
        int getPortNum() const { return port; }
        virtual bool operator == (const PeerAddressImpl &a) const {
            const UDPPeerAddressImpl &comp = dynamic_cast<const UDPPeerAddressImpl &>(a);
            return (comp.port == port) && (comp.address == address);
        }
    private:
        SocketAddress address;
        int port;
    };
    
    class UDPPeerAddress : public PeerAddress {
    public:
        UDPPeerAddress(PeerAddressImpl *impl) : PeerAddress(impl) {}
        UDPPeerAddress(const PeerAddress &a) : PeerAddress(a) {}
        UDPPeerAddress(const SocketAddress &addr, int portNum);
        SocketAddress getSocketAddress() const;
        int getPortNum() const;
    };
}

#endif