namespace ios_fc {

class PeerAddress {
public:
    virtual ~PeerAddress() {}
};

class Dirigeable {
public:
    virtual PeerAddress getPeerAddress() = 0;
    virtual PeerAddress getBroadcastAddress() = 0;
    virtual void setPeerAddress(PeerAddress) = 0;
};

}
