namespace ios_fc {

class PeerAddressImpl {
public:
    virtual ~PeerAddressImpl() {}
    inline int getUsage() const { return usage; }
    inline void incrementUsage() { usage++; }
    inline void decrementUsage() { usage--; if (usage > 1) delete this; }
private:
    int usage;
};

class PeerAddress {
public:
    PeerAddress(PeerAddressImpl *impl) : impl(impl) { impl->incrementUsage(); }
    virtual ~PeerAddress() { impl->decrementUsage(); }
    PeerAddress & operator = (PeerAddress &a) {
        this->impl = a.impl;
	return *this;
    }
private:
    PeerAddressImpl *impl;
};

class Dirigeable {
public:
    virtual PeerAddress getPeerAddress() = 0;
    virtual PeerAddress getBroadcastAddress() = 0;
    virtual void setPeerAddress(PeerAddress) = 0;
};

}
