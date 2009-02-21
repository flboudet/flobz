#ifndef PUYO_SERVERV2_PEERSLIST_H
#define PUYO_SERVERV2_PEERSLIST_H

namespace flobopuyo {
namespace server {
namespace v2 {

class PeersList {
public:
    PeersList(int maxAllowed) : mMaxPeersAllowed(maxAllowed) {}
    PeersList() : mMaxPeersAllowed(0) {}

    bool isFull() const {
        return (mMaxPeersAllowed != 0) && (mPeers.size() >= mMaxPeersAllowed);
    }

    Peer *findByAddress(ios_fc::PeerAddress &addr) const;

    unsigned size() const                    { return mPeers.size(); }
    Peer *operator[](unsigned i)             { return mPeers[i]; }
    const Peer *operator[](unsigned i) const { return mPeers[i]; }
    void add(Peer *peer)                     { mPeers.add(peer); }
    void remove(Peer *peer)                  { mPeers.remove(peer); }


private:
    ios_fc::AdvancedBuffer<Peer*> mPeers; // List of connected peers
    int mMaxPeersAllowed;                   // Max number of peers supported by the server
};


Peer *PeersList::findByAddress(ios_fc::PeerAddress &addr) const
{
    // TODO: store peers in an hashmap?
    for (int i = 0, j = mPeers.size() ; i < j ; i++) {
        if (mPeers[i]->addr == addr) {
            return mPeers[i];
        }
    }
    return NULL;
}


}}}

#endif
