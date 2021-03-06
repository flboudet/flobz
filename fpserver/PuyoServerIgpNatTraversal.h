/* -*-mode:c; c-style:k&r; c-basic-offset:4; indent-tab-mode: nil; -*- */
#ifndef _PUYOSERVERIGPNATTRAVERSAL_H
#define _PUYOSERVERIGPNATTRAVERSAL_H

#include "ios_igpvirtualpeermessagebox.h"
#include "ios_igpmessagelistener.h"
#include "ios_memory.h"
#include "ios_dirigeable.h"

using namespace ios_fc;

class PuyoIgpNatTraversal : public MessageListener {
public:
    PuyoIgpNatTraversal(MessageBox &mbox, IgpMessageListener &listener) : mbox(mbox), igpListener(listener), timeMsBeforePunchTimeout(5000.) {}
    void onMessage(Message &msg);
    void idle();
private:
    class PunchPool;
    void updatePeer(PeerAddress addr, const String name, int status);

    MessageBox &mbox;
    IgpMessageListener &igpListener;
    double timeMsBeforePunchTimeout;
    AdvancedBuffer<PunchPool *> pools;
};

#endif // _PUYOSERVERIGPNATTRAVERSAL_H

