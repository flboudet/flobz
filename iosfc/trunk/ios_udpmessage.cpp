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

UDPMessage::~UDPMessage()
{
}

void UDPMessage::sendBuffer(Buffer<char> out) const
{
    owner.sendUDP(out, getSerialID(), isReliable(), peerAddressImpl->getAddress(), peerAddressImpl->getPortNum());
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
    return peerAddress;
}

void UDPMessage::setPeerAddress(PeerAddress newPeerAddress)
{
    UDPPeerAddressImpl *newPeerAddressImpl = dynamic_cast<UDPPeerAddressImpl *>(newPeerAddress.getImpl());
    if (newPeerAddressImpl != NULL) {
        peerAddress = newPeerAddress;
    }
    else throw Exception("Incompatible peer address type!");
}

};
