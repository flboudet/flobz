#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_udpmessage.h"

namespace ios_fc {

UDPPeerAddress::UDPPeerAddress(const SocketAddress &addr, int portNum)
    : PeerAddress(new UDPPeerAddressImpl(addr, portNum))
{
}

SocketAddress UDPPeerAddress::getSocketAddress() const
{
    UDPPeerAddressImpl *peerAddressImpl = dynamic_cast<UDPPeerAddressImpl *>(getImpl());
    if (peerAddressImpl != NULL) {
        return peerAddressImpl->getAddress();
    }
    else throw Exception("Incompatible peer address type!");
}

int UDPPeerAddress::getPortNum() const
{
    UDPPeerAddressImpl *peerAddressImpl = dynamic_cast<UDPPeerAddressImpl *>(getImpl());
    if (peerAddressImpl != NULL) {
        return peerAddressImpl->getPortNum();
    }
    else throw Exception("Incompatible peer address type!");
}

}
