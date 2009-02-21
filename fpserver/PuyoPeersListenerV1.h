#ifndef _PUYOSERVERIGPRESPONDER_H
#define _PUYOSERVERIGPRESPONDER_H

#include "ios_igpvirtualpeermessagebox.h"
#include "ios_memory.h"
#include "ios_dirigeable.h"

using namespace ios_fc;

class PuyoIgpResponder : public MessageListener {
public:
    PuyoIgpResponder(IgpVirtualPeerMessageBox &mbox);
    bool checkVersion(int clientVersion) const;
    void onMessage(Message &msg);
    void idle();
private:
    class GamePeer;
    
    GamePeer * getPeer(PeerAddress &addr) const;
    void connectPeer(PeerAddress addr, int fpipVersion, const String name, int status);
    void updatePeer(GamePeer *peer, int status);

    IgpVirtualPeerMessageBox &mbox;
    AdvancedBuffer<GamePeer *>peers;
    double timeMsBeforePeerTimeout;
    int m_maxPeersAllowed;
};

class PuyoIgpResponder::GamePeer {
public:
    GamePeer(PeerAddress addr, const String name) : addr(addr), name(name), status(0) {}
    PeerAddress addr;
    const String name;
    double lastUpdate;
    int status;
};

#endif // _PUYOSERVERIGPRESPONDER_H

