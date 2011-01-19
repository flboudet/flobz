#include "ios_memory.h"
#include "ios_hash.h"
#include "ios_udpmessage.h"

namespace ios_fc {

UDPPeerAddress::UDPPeerAddress(const SocketAddress &addr, int portNum)
    : PeerAddress(new ios_fc::_private_udpmessagebox::UDPPeerAddressImpl(addr, portNum))
{
}

SocketAddress UDPPeerAddress::getSocketAddress() const
{
    ios_fc::_private_udpmessagebox::UDPPeerAddressImpl *peerAddressImpl = dynamic_cast<ios_fc::_private_udpmessagebox::UDPPeerAddressImpl *>(getImpl());
    if (peerAddressImpl != NULL) {
        return peerAddressImpl->getAddress();
    }
    else throw Exception("Incompatible peer address type!");
}

int UDPPeerAddress::getPortNum() const
{
    ios_fc::_private_udpmessagebox::UDPPeerAddressImpl *peerAddressImpl = dynamic_cast<ios_fc::_private_udpmessagebox::UDPPeerAddressImpl *>(getImpl());
    if (peerAddressImpl != NULL) {
        return peerAddressImpl->getPortNum();
    }
    else throw Exception("Incompatible peer address type!");
}

}
