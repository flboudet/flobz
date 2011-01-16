#ifndef _IOS_UDP_MESSAGE_H
#define _IOS_UDP_MESSAGE_H

#include "ios_message.h"
#include "ios_udpmessagebox.h"
#include "ios_dirigeable.h"

namespace ios_fc {

static const char *SERIAL_ID   = "SID";
static const char *IS_RELIABLE = "RELIABLE";

class UDPPeerAddress : public PeerAddress {
public:
    UDPPeerAddress(PeerAddressImpl *impl) : PeerAddress(impl) {}
    UDPPeerAddress(const PeerAddress &a) : PeerAddress(a) {}
    UDPPeerAddress(const SocketAddress &addr, int portNum);
    SocketAddress getSocketAddress() const;
    int getPortNum() const;
};

class UDPPeerAddressImpl : public PeerAddressImpl {
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

template <typename T>
class UDPMessage : public T, public Dirigeable {
public:
    UDPMessage(int serialID, UDPMessageBox &owner, SocketAddress address, int port);
    UDPMessage(int serialID, UDPMessageBox &owner, const PeerAddress &address);
    UDPMessage(const VoidBuffer &serialized, UDPMessageBox &owner, SocketAddress address, int port) throw(Message::InvalidMessageException);
    virtual ~UDPMessage() {}
    // Message implementation
    virtual void send();
    // Dirigeable implementation
    PeerAddress getPeerAddress()
    { return peerAddress; }
    PeerAddress getPeerAddress(const String key);
    PeerAddress getBroadcastAddress();
    void setPeerAddress(PeerAddress);
    void addPeerAddress(const String key, const PeerAddress &value);
    // Own methods
    int  getSerialID() const;
    bool isReliable() const;
private:
    UDPMessageBox &owner;
    UDPPeerAddressImpl *peerAddressImpl;
    PeerAddress peerAddress;

    UDPMessage(const UDPMessage&);
    UDPMessage&operator=(const UDPMessage&);
};

template <typename T>
UDPMessage<T>::UDPMessage(int serialID, UDPMessageBox &owner, SocketAddress address, int port)
    : owner(owner),
      peerAddressImpl(new UDPPeerAddressImpl(address, port)),
      peerAddress(peerAddressImpl)
{
    T::addIntProperty(SERIAL_ID, serialID);
}

template <typename T>
UDPMessage<T>::UDPMessage(int serialID, UDPMessageBox &owner, const PeerAddress &address)
    : owner(owner),
      peerAddressImpl(static_cast<UDPPeerAddressImpl *>(address.getImpl())),
      peerAddress(address)
{
    T::addIntProperty(SERIAL_ID, serialID);
}

template <typename T>
UDPMessage<T>::UDPMessage(const VoidBuffer &serialized, UDPMessageBox &owner, SocketAddress address, int port)  throw(Message::InvalidMessageException)
: T(serialized), owner(owner),
  peerAddressImpl(new UDPPeerAddressImpl(address, port)),
  peerAddress(peerAddressImpl)
{
}

template <typename T>
void UDPMessage<T>::send()
{
    VoidBuffer out = T::serialize();
    owner.sendUDP(out, getSerialID(), isReliable(), peerAddress, peerAddressImpl->getAddress(), peerAddressImpl->getPortNum());
}

template <typename T>
PeerAddress UDPMessage<T>::getPeerAddress(const String key)
{
    return PeerAddress(new UDPPeerAddressImpl(SocketAddress(T::getString(key + "_A")), T::getInt(key + "_PN")));
}

template <typename T>
PeerAddress UDPMessage<T>::getBroadcastAddress()
{
    return PeerAddress(new UDPPeerAddressImpl(owner.getBroadcastAddress(), peerAddressImpl->getPortNum()));
}

template <typename T>
void UDPMessage<T>::setPeerAddress(PeerAddress newPeerAddress)
{
    UDPPeerAddressImpl *newPeerAddressImpl = dynamic_cast<UDPPeerAddressImpl *>(newPeerAddress.getImpl());
    if (newPeerAddressImpl != NULL) {
        peerAddress = newPeerAddress;
        peerAddressImpl = newPeerAddressImpl;
    }
    else throw Exception("Incompatible peer address type!");
}

template <typename T>
void UDPMessage<T>::addPeerAddress(const String key, const PeerAddress &value)
{
    UDPPeerAddressImpl *peerAddressImpl = dynamic_cast<UDPPeerAddressImpl *>(value.getImpl());
    if (peerAddressImpl != NULL) {
        T::addInt(key + "_PN", peerAddressImpl->getPortNum());
        T::addString(key + "_A", peerAddressImpl->getAddress().asString());
    }
    else throw Exception("Incompatible peer address type!");
}

template <typename T>
int UDPMessage<T>::getSerialID() const
{
    return T::getIntProperty(SERIAL_ID);
}

template <typename T>
bool UDPMessage<T>::isReliable() const
{
    if (T::hasBoolProperty(IS_RELIABLE))
        return T::getBoolProperty(IS_RELIABLE);
    return false;
}

}

#endif
