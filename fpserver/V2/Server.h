#ifndef PUYO_SERVERV2_SERVER_H
#define PUYO_SERVERV2_SERVER_H

namespace flobopuyo {
namespace server {
namespace v2 {

class Server {
public:
    Server(ios_fc::IgpVirtualPeerMessageBox &mbox) : mMbox(mbox), mTimeMsBeforePeerTimeout(5000.) {}

    // Process alive messages
    void onIgpAlive(ios_fc::Message &msg, ios_fc::PeerAddress &address);
    // Process chat messages
    void onIgpChat(ios_fc::Message &msg);

    // Called when a peer connects
    void onPeerConnect(ios_fc::PeerAddress addr, int fpipVersion, const ios_fc::String name, int status);
    // Called when a peer updates its status
    void onPeerUpdate(Peer *peer, int status);
    // Called when a peer is disconnected
    void onPeerDisconnect(Peer *peer);

    // Check if a peer reached timeout.
    void checkTimeouts();

    PeersList mPeers;
    ios_fc::IgpVirtualPeerMessageBox &mMbox; // Message box
    double mTimeMsBeforePeerTimeout;         // Peers are disconnected when last update gets older than this
};

void Server::checkTimeouts() {
    double time_ms = ios_fc::getTimeMs();
    for (int i = 0; i < mPeers.size() ; i++) {
        Peer *currentPeer = mPeers[i];
        if ((time_ms - currentPeer->lastUpdate) > mTimeMsBeforePeerTimeout) {
#if DEBUG_PUYOPEERSLISTENERV2
            printf("PuyoServer Peer timeout!\n");
#endif
            onPeerDisconnect(currentPeer);
            // Peer vector have been modified, stop looping it.
            break;
        }
    }
}

void Server::onIgpAlive(ios_fc::Message &msg, ios_fc::PeerAddress &address)
{
    Peer *currentPeer = mPeers.findByAddress(address);
    if (currentPeer != NULL) { // Already know? 
        int status = msg.getInt("STATUS");
        onPeerUpdate(currentPeer, status);
    }
    else { // No such peer? Connect it
        int protocolVersion = msg.getInt("V"); // version of the FPIP protocol used by the client
        int status = msg.getInt("STATUS");
        ios_fc::String name = msg.getString("NAME");
        onPeerConnect(address, protocolVersion, name, status);
    }
}

void Server::onIgpChat(ios_fc::Message &msg)
{
#if DEBUG_PUYOPEERSLISTENERV2
    printf("Message de %s: %s\n", (const char *)msg.getString("NAME"), (const char *)msg.getString("MSG"));
#endif
    ios_fc::Message *newMsg = mMbox.createMessage();
    try {
        newMsg->addBoolProperty("RELIABLE", true);
        newMsg->addInt("CMD", PUYO_IGP_CHAT);
        newMsg->addString("NAME", msg.getString("NAME"));
        newMsg->addString("MSG", msg.getString("MSG"));
        ios_fc::Dirigeable *dirNew = dynamic_cast<ios_fc::Dirigeable *>(newMsg);
        for (int i = 0, j = mPeers.size() ; i < j ; i++) {
#if DEBUG_PUYOPEERSLISTENERV2
            printf("Diffusion au peer num %d\n", i);
#endif
            dirNew->setPeerAddress(mPeers[i]->addr);
            newMsg->send();
        }
    } catch (ios_fc::Exception e) {}
    delete newMsg;
}

void Server::onPeerConnect(ios_fc::PeerAddress addr, int fpipVersion, const ios_fc::String name, int status)
{
    ConnectionRequest request(mPeers, addr, fpipVersion, name, status);
    bool accept = request.isAcceptable();

    if (accept) {
        // Send accept message
        ios_fc::Message *acceptMsg = mMbox.createMessage();
        acceptMsg->addBoolProperty("RELIABLE", true);
        acceptMsg->addInt("CMD", PUYO_IGP_ACCEPT);
        ios_fc::Dirigeable *dirAccept = dynamic_cast<ios_fc::Dirigeable *>(acceptMsg);
        dirAccept->setPeerAddress(addr);
        acceptMsg->send();
        delete acceptMsg;
        
        // Create the new peer
        Peer *newPeer = new Peer(addr, name);
        newPeer->lastUpdate = ios_fc::getTimeMs();
#if DEBUG_PUYOPEERSLISTENERV2
        printf("Nouveau peer: %s\n", (const char *)name);
#endif
        
        // Envoyer tous les peers connectes au petit nouveau
        for (int i = 0; i < mPeers.size(); i++) {
            ios_fc::Message *newMsg = mMbox.createMessage();
            newMsg->addBoolProperty("RELIABLE", true);
            newMsg->addInt("CMD", PUYO_IGP_CONNECT);
            newMsg->addString("NAME", mPeers[i]->name);
            newMsg->addInt("STATUS", mPeers[i]->status);
            ios_fc::Dirigeable *dirNew = dynamic_cast<ios_fc::Dirigeable *>(newMsg);
            dirNew->addPeerAddress("ADDR", mPeers[i]->addr);
            dirNew->setPeerAddress(addr);
            newMsg->send();
            delete newMsg;
        }
        // Inserer le petit nouveau a la liste
        mPeers.add(newPeer);
        
        // Envoyer l'info de connexion a tous les peers
        ios_fc::Message *newMsg = mMbox.createMessage();
        newMsg->addBoolProperty("RELIABLE", true);
        newMsg->addInt("CMD", PUYO_IGP_CONNECT);
        newMsg->addString("NAME", name);
        newMsg->addInt("STATUS", status);
        ios_fc::Dirigeable *dirNew = dynamic_cast<ios_fc::Dirigeable *>(newMsg);
        dirNew->addPeerAddress("ADDR", addr);
        for (int i = 0; i < mPeers.size(); i++) {
#if DEBUG_PUYOPEERSLISTENERV2
            printf("Diffusion connexion au peer num %d\n", i);
#endif
            dirNew->setPeerAddress(mPeers[i]->addr);
            newMsg->send();
        }
        delete newMsg;
    }
    else {
        // Send deny message
        ios_fc::Message *denyMsg = mMbox.createMessage();
        denyMsg->addBoolProperty("RELIABLE", true);
        denyMsg->addInt("CMD", PUYO_IGP_DENY);
        denyMsg->addString("MSG", request.getDenyErrorString());
        ios_fc::Dirigeable *dirDeny = dynamic_cast<ios_fc::Dirigeable *>(denyMsg);
        dirDeny->setPeerAddress(addr);
        denyMsg->send();
        delete denyMsg;
    }
}

void Server::onPeerUpdate(Peer *peer, int status)
{
    peer->lastUpdate = ios_fc::getTimeMs();
    if (peer->status != status) {
        peer->status = status;
        // Send a STATUSCHANGE message to the other peers
        ios_fc::Message *newMsg = mMbox.createMessage();
        newMsg->addBoolProperty("RELIABLE", true);
        newMsg->addInt("CMD", PUYO_IGP_STATUSCHANGE);
        newMsg->addString("NAME", peer->name);
        newMsg->addInt("STATUS", status);
        ios_fc::Dirigeable *dirNew = dynamic_cast<ios_fc::Dirigeable *>(newMsg);
        dirNew->addPeerAddress("ADDR", peer->addr);
        for (int i = 0; i < mPeers.size(); i++) {
#if DEBUG_PUYOPEERSLISTENERV2
            printf("Diffusion connexion au peer num %d\n", i);
#endif
            dirNew->setPeerAddress(mPeers[i]->addr);
            newMsg->send();
        }
        delete newMsg;
    }
}

void Server::onPeerDisconnect(Peer *peer) {
    // Build disconnect message
    ios_fc::Message *newMsg = mMbox.createMessage();
    newMsg->addBoolProperty("RELIABLE", true);
    newMsg->addInt("CMD", PUYO_IGP_DISCONNECT);
    newMsg->addString("NAME", peer->name);
    ios_fc::Dirigeable *dirNew = dynamic_cast<ios_fc::Dirigeable *>(newMsg);
    dirNew->addPeerAddress("ADDR", peer->addr);
    // Delete peer
    mPeers.remove(peer);
    delete peer;
    // Warn the others
    for (int i = 0; i < mPeers.size(); i++) {
        dirNew->setPeerAddress(mPeers[i]->addr);
        newMsg->send();
    }
    delete newMsg;
}

}}}

#endif
