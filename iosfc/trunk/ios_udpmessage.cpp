#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_udpmessage.h"

namespace ios_fc {

class UDPMessage::UDPPeerAddressImpl : public PeerAddressImpl {
public:
    UDPPeerAddressImpl(SocketAddress address, int port)
      : address(address), port(port) {}
private:
    SocketAddress address;
    int port;
};

UDPMessage::UDPMessage(int serialID, UDPMessageBox &owner, SocketAddress address, int port)
    : StandardMessage(serialID), owner(owner),
      peerAddress(new UDPPeerAddressImpl(address, port))
{
}

UDPMessage::~UDPMessage()
{
}

void UDPMessage::sendBuffer(Buffer<char> out) const
{
    owner.sendUDP(out, getSerialID(), isReliable());
}

UDPMessage::UDPMessage(const Buffer<char> raw, UDPMessageBox &owner, SocketAddress address, int port)  throw(InvalidMessageException)
: StandardMessage(raw), owner(owner),
  peerAddress(new UDPPeerAddressImpl(address, port))
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
    peerAddress = newPeerAddress;
}

};
