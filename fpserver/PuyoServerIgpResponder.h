#ifndef _PUYOSERVERIGPRESPONDER_H
#define _PUYOSERVERIGPRESPONDER_H

#include "ios_igpvirtualpeermessagebox.h"
#include "ios_memory.h"
#include "ios_dirigeable.h"

using namespace ios_fc;

class PuyoIgpResponder : public MessageListener {
public:
    PuyoIgpResponder(IgpVirtualPeerMessageBox &mbox) : mbox(mbox), timeMsBeforePeerTimeout(5000.) {}
    void onMessage(Message &msg);
    void idle();
private:
    void updatePeer(PeerAddress addr, const String name);

    IgpVirtualPeerMessageBox &mbox;
    class GamePeer;
    AdvancedBuffer<GamePeer *>peers;
    double timeMsBeforePeerTimeout;
};

class PuyoIgpResponder::GamePeer {
public:
    GamePeer(PeerAddress addr, const String name) : addr(addr), name(name) {}
    PeerAddress addr;
    const String name;
    double lastUpdate;
};

#endif // _PUYOSERVERIGPRESPONDER_H

