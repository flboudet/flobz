#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_udpmessage.h"

namespace ios_fc {

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

UDPMessage::UDPMessage(int serialID, UDPMessageBox &owner, SocketAddress address, int port)
    : StandardMessage(serialID), owner(owner),
      peerAddressImpl(new UDPPeerAddressImpl(address, port)),
      peerAddress(peerAddressImpl)
{
}

UDPMessage::UDPMessage(int serialID, UDPMessageBox &owner, const PeerAddress &address)
    : StandardMessage(serialID), owner(owner),
      peerAddress(address)
{
    peerAddressImpl = static_cast<UDPPeerAddressImpl *>(address.getImpl());
}

UDPMessage::~UDPMessage()
{
}

void UDPMessage::sendBuffer(Buffer<char> out) const
{
    owner.sendUDP(out, getSerialID(), isReliable(), peerAddress, peerAddressImpl->getAddress(), peerAddressImpl->getPortNum());
}

UDPMessage::UDPMessage(const Buffer<char> raw, UDPMessageBox &owner, SocketAddress address, int port)  throw(InvalidMessageException)
: StandardMessage(raw), owner(owner),
  peerAddressImpl(new UDPPeerAddressImpl(address, port)),
  peerAddress(peerAddressImpl)
{
}

PeerAddress UDPMessage::getPeerAddress()
{
    return peerAddress;
}

PeerAddress UDPMessage::getBroadcastAddress()
{
    return PeerAddress(new UDPPeerAddressImpl(owner.getBroadcastAddress(), peerAddressImpl->getPortNum()));
}

void UDPMessage::setPeerAddress(PeerAddress newPeerAddress)
{
    UDPPeerAddressImpl *newPeerAddressImpl = dynamic_cast<UDPPeerAddressImpl *>(newPeerAddress.getImpl());
    if (newPeerAddressImpl != NULL) {
        peerAddress = newPeerAddress;
        peerAddressImpl = newPeerAddressImpl;
    }
    else throw Exception("Incompatible peer address type!");
}

void UDPMessage::addPeerAddress(const String key, const PeerAddress &value)
{
    UDPPeerAddressImpl *peerAddressImpl = dynamic_cast<UDPPeerAddressImpl *>(value.getImpl());
    if (peerAddressImpl != NULL) {
        addInt(key + "_PN", peerAddressImpl->getPortNum());
        addString(key + "_A", peerAddressImpl->getAddress().asString());
    }
    else throw Exception("Incompatible peer address type!");
}

PeerAddress UDPMessage::getPeerAddress(const String key)
{
    return PeerAddress(new UDPPeerAddressImpl(SocketAddress(getString(key + "_A")), getInt(key + "_PN")));
}

SocketAddress UDPPeerAddress::getSocketAddress() const
{
    UDPMessage::UDPPeerAddressImpl *peerAddressImpl = dynamic_cast<UDPMessage::UDPPeerAddressImpl *>(getImpl());
    if (peerAddressImpl != NULL) {
        return peerAddressImpl->getAddress();
    }
    else throw Exception("Incompatible peer address type!");
}

int UDPPeerAddress::getPortNum() const
{
    UDPMessage::UDPPeerAddressImpl *peerAddressImpl = dynamic_cast<UDPMessage::UDPPeerAddressImpl *>(getImpl());
    if (peerAddressImpl != NULL) {
        return peerAddressImpl->getPortNum();
    }
    else throw Exception("Incompatible peer address type!");
}
        
};
