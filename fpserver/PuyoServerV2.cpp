/* -*-mode:c; c-style:k&r; c-basic-offset:4; indent-tab-mode: nil; -*- */
#include "PuyoServerV2.h"

#define DEBUG_PUYOPEERSLISTENERV2 1

#include <stdio.h>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <sstream>

#include "ios_igpmessagebox.h"
#include "ios_memory.h"
#include "ios_dirigeable.h"
#include "ios_time.h"
#include "PuyoIgpDefs.h"

#include "V2/Peer.h"

#include "V2/messages/AcceptMessage.h"
#include "V2/messages/ConnectMessage.h"
#include "V2/messages/DenyMessage.h"

#include "V2/Database.h"
#include "V2/PeersList.h"
#include "V2/ConnectionRequest.h"
#include "V2/Server.h"

using namespace ios_fc;
namespace flobopuyo {
namespace server {

// ----------------
// Public interface
// ----------------
PuyoServerV2::PuyoServerV2(ios_fc::MessageBox &mbox)
 : server(new v2::Server(mbox))
{}

PuyoServerV2::~PuyoServerV2() {
    delete server;
}

void PuyoServerV2::onMessage(Message &msg) {
    // We let the messagebox handle the exception because its
    // default behaviour is to disconnect the peer, which is fine.
    // try {}

    // Ohoh! What is this strange message?
    if (!msg.hasInt("CMD")) return;

    // Gets the address of the sender of the message
    Dirigeable &dirMsg = dynamic_cast<Dirigeable &>(msg);
    PeerAddress address = dirMsg.getPeerAddress();

    switch (msg.getInt("CMD")) {
        // A client will send an alive message periodically to inform the server it is still connected
        case PUYO_IGP_ALIVE:
            server->onIgpAlive(msg, address);
            break;
            // A client will send a chat message to communicate with the other connected clients
        case PUYO_IGP_CHAT:
            server->onIgpChat(msg);
            break;
        case PUYO_IGP_GAME_OVER:
            server->onIgpGameOver(msg);
            break;
        default:
            break;
    }
}
 
void PuyoServerV2::idle()
{
    server->checkTimeouts();
}


}} // End of namespace flobopuyo::server

