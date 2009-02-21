#ifndef _PUYOPEERSLISTENERV2_H
#define _PUYOPEERSLISTENERV2_H

#include "PuyoPeersListener.h"

namespace flobopuyo {
namespace server {

class PuyoPeersListenerV2 : public PuyoPeersListener {
public:
    PuyoPeersListenerV2(ios_fc::IgpVirtualPeerMessageBox &mbox);
    bool checkVersion(int clientVersion) const;
    void onMessage(ios_fc::Message &msg);
    void idle();
private:
    class GamePeer;
    
    GamePeer * getPeer(ios_fc::PeerAddress &addr) const;
    void connectPeer(ios_fc::PeerAddress addr, int fpipVersion, const ios_fc::String name, int status);
    void updatePeer(GamePeer *peer, int status);

    ios_fc::IgpVirtualPeerMessageBox &mbox;
    ios_fc::AdvancedBuffer<GamePeer *>peers;
    double timeMsBeforePeerTimeout;
    int m_maxPeersAllowed;
};

class PuyoPeersListenerV2::GamePeer {
public:
    GamePeer(ios_fc::PeerAddress addr, const ios_fc::String name) : addr(addr), name(name), status(0) {}
    ios_fc::PeerAddress addr;
    const ios_fc::String name;
    double lastUpdate;
    int status;
};

}}

#endif
