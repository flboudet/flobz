#ifndef _PUYOSERVERIGPRESPONDER_H
#define _PUYOSERVERIGPRESPONDER_H

#include "PuyoServer.h"

namespace flobopuyo {
namespace server {

class PuyoServerV1 : public PuyoServer {
public:
    PuyoServerV1(ios_fc::MessageBox &mbox);
    bool checkVersion(int clientVersion) const;
    void onMessage(ios_fc::Message &msg);
    void idle();
private:
    class GamePeer;
    
    GamePeer * getPeer(ios_fc::PeerAddress &addr) const;
    void connectPeer(ios_fc::PeerAddress addr, int fpipVersion, const ios_fc::String name, int status);
    void updatePeer(GamePeer *peer, int status);

    ios_fc::MessageBox &mbox;
    ios_fc::AdvancedBuffer<GamePeer *>peers;
    double timeMsBeforePeerTimeout;
    int m_maxPeersAllowed;
};

class PuyoServerV1::GamePeer {
public:
    GamePeer(ios_fc::PeerAddress addr, const ios_fc::String name) : addr(addr), name(name), status(0) {}
    ios_fc::PeerAddress addr;
    const ios_fc::String name;
    double lastUpdate;
    int status;
};

}}

#endif // _PUYOSERVERIGPRESPONDER_H

