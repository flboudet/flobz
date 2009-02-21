#ifndef _PUYOPEERSLISTENERV2_H
#define _PUYOPEERSLISTENERV2_H

#include "PuyoServer.h"

namespace flobopuyo {
namespace server {

namespace v2 {
    class Server;
}

class PuyoServerV2 : public PuyoServer {

public:
    PuyoServerV2(ios_fc::IgpVirtualPeerMessageBox &mbox);
    ~PuyoServerV2();
    void onMessage(ios_fc::Message &msg);
    void idle();

private:
    v2::Server *server;
};

}}

#endif
