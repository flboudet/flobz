namespace ios_fc {

class PeerAddressImpl {
public:
    PeerAddressImpl() : usage(0) {}
    virtual ~PeerAddressImpl() {}
    virtual bool operator == (const PeerAddressImpl &) const = 0;
    inline int getUsage() const { return usage; }
    inline void incrementUsage() { usage++; }
    inline void decrementUsage() {
        usage--;
        if (usage < 1) {
            delete this;
        }
    }
private:
    int usage;
};

class PeerAddress {
public:
    PeerAddress() : impl(NULL) {}
    PeerAddress(PeerAddressImpl *impl) : impl(impl) { impl->incrementUsage(); }
    PeerAddress(const PeerAddress &a) : impl(a.impl) { impl->incrementUsage(); }
    virtual ~PeerAddress() { impl->decrementUsage(); }
    PeerAddress & operator = (const PeerAddress &a) {
        if (impl != NULL) impl->decrementUsage();
        this->impl = a.impl;
        impl->incrementUsage();
        return *this;
    }
    bool operator == (const PeerAddress &a) const {
        return (*impl == *(a.impl));
    } 
    inline PeerAddressImpl *getImpl() const { return impl; }
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
