#ifndef PUYO_SERVERV2_SERVER_H
#define PUYO_SERVERV2_SERVER_H

namespace flobopuyo {
namespace server {
namespace v2 {

struct GameResultInPool {
    int gameid;
    String name1;
    String name2;
    int winner;
    int explode_count;
    int drop_count;
    bool is_dead;
    bool is_winner;
};

class GameResultPool : public std::vector<GameResultInPool> {
public:
    enum SimilarResult {
        SIMILAR,
        SIMILAR_BUT_SUSPECT,
        NO_SIMILAR
    };
    Similarity contains(const GameResultInPool &result) const;
};

class Server {
public:
    Server(ios_fc::IgpVirtualPeerMessageBox &mbox) : mMbox(mbox), mTimeMsBeforePeerTimeout(5000.) {}

    // Process alive messages
    void onIgpAlive(ios_fc::Message &msg, ios_fc::PeerAddress &address);
    // Process chat messages
    void onIgpChat(ios_fc::Message &msg);
    // Check if a peer reached timeout.
    void checkTimeouts();
    // A game is over
    void onIgpGameOver(ios_fc::Message &msg);

private:
    // Called when a peer connects
    void onPeerConnect(ios_fc::PeerAddress addr, int fpipVersion, const ios_fc::String name, const ios_fc::String password, int status);
    // Called when a peer updates its status
    void onPeerUpdate(Peer *peer, int status);
    // Called when a peer is disconnected
    void onPeerDisconnect(Peer *peer);

    Database mDB;
    PeersList mPeers;
    ios_fc::IgpVirtualPeerMessageBox &mMbox; // Message box
    double mTimeMsBeforePeerTimeout;         // Peers are disconnected when last update gets older than this
};

void Server::checkTimeouts() {
    double time_ms = ios_fc::getTimeMs();
    for (int i = 0; i < mPeers.size() ; i++) {
        Peer *currentPeer = mPeers[i];
        if (currentPeer->isTimeout(time_ms)) {
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
#if DEBUG_PUYOPEERSLISTENERV2
        printf("onIgpAlive: No such peer? Connect it\n");
#endif
        int protocolVersion = msg.getInt("V"); // version of the FPIP protocol used by the client
        int status = msg.getInt("STATUS");
        ios_fc::String name = msg.getString("NAME");
        printf("Name = %s\n", (const char*)name);
        ios_fc::String pass;
        if (msg.hasString("PASSWD"))
            pass = msg.getString("PASSWD");
        else
            pass = "";
        onPeerConnect(address, protocolVersion, name, pass, status);
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
    } catch (ios_fc::Exception e) {
        // TODO: something here
    }
    delete newMsg;
}

void Server::onPeerConnect(ios_fc::PeerAddress addr, int fpipVersion, const ios_fc::String name, const ios_fc::String password, int status)
{
    ConnectionRequest request(mDB, mPeers, addr, fpipVersion, name, password, status);
    bool accept = request.isAcceptable();

    if (accept) {
        // Send accept message
        AcceptMessage acceptMsg(mMbox);
        acceptMsg.sendTo(addr);
        
        // Create the new peer
        Peer *newPeer = new Peer(addr, name, mTimeMsBeforePeerTimeout);

#if DEBUG_PUYOPEERSLISTENERV2
        printf("Nouveau peer: %s\n", (const char *)name);
#endif
        mDB.storeConnection(name.c_str());
        mDB.loadPeerInfos(newPeer);
        
        // Envoyer tous les peers connectes au petit nouveau
        for (int i = 0; i < mPeers.size(); i++) {
            ConnectMessage newMsg(mMbox, mPeers[i]);
            newMsg.sendTo(addr);
        }
        // Inserer le petit nouveau a la liste
        mPeers.add(newPeer);
        
        // Envoyer l'info de connexion a tous les peers
        ConnectMessage newMsg(mMbox, newPeer);
        for (int i = 0; i < mPeers.size(); i++) {
#if DEBUG_PUYOPEERSLISTENERV2
            printf("Diffusion connexion au peer num %d\n", i);
#endif
            newMsg.sendTo(mPeers[i]->addr);
        }
    }
    else {
        // Send deny message
#if DEBUG_PUYOPEERSLISTENERV2
        printf("Connection refused for %s (%s)\n", (const char*)name, request.getDenyErrorString().c_str());
#endif
        DenyMessage denyMsg(mMbox, request.getDenyErrorString(), request.getDenyErrorStringMore());
        denyMsg.sendTo(addr);
    }
}

void Server::onPeerUpdate(Peer *peer, int status)
{
    peer->touch();

    if (peer->status != status) {
        peer->status = status;
        // Send a STATUSCHANGE message to the other peers
        ios_fc::Message *newMsg = mMbox.createMessage();
        newMsg->addBoolProperty("RELIABLE", true);
        newMsg->addInt("CMD", PUYO_IGP_STATUSCHANGE);
        newMsg->addString("NAME", peer->name);
        newMsg->addInt("RANK", peer->rank);
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

struct PlayerGameStat
{
    int combo_count[24]; //
    int explode_count; //
    int drop_count; //
    int ghost_sent_count; //
    double time_left;
    bool is_dead;
    bool is_winner;
    int points; //
    int total_points;
};

void Server::onIgpGameOver(ios_fc::Message &msg) {
    int winner = msg.getInt("WINNER");
    int gameId = msg.getInt("GAMEID");
    ios_fc::String name1 = msg.getString("NAME1");
    ios_fc::String name2 = msg.getString("NAME2");
    PlayerGameStat gameStat;
    gameStat.points = msg.getInt("SCORE");
    gameStat.total_points = msg.getInt("TOTAL_SCORE");
    for (int i = 0 ; i < 24 ; i++) {
        ios_fc::String messageName = ios_fc::String("COMBO_COUNT") + i;
        gameStat.combo_count[i] = msg.getInt(messageName);
    }
    gameStat.explode_count = msg.getInt("EXPLODE_COUNT");
    gameStat.drop_count = msg.getInt("DROP_COUNT");
    gameStat.ghost_sent_count = msg.getInt("GHOST_SENT_COUNT");
    gameStat.time_left = msg.getFloat("TIME_LEFT");
    gameStat.is_dead = msg.getBool("IS_DEAD");
    gameStat.is_winner = msg.getBool("IS_WINNER");

    // Store the result into a temporary pool.
    // If the pool already contains a result for this game (sent by the other player),
    // then log the result.
    // Except if the result are different (the 2 players claims victory).
    // In this case, give penalty points to both... (p = p*2 + 1)
    printf("Game %d with %s and %s won by %d\n", gameId, name1.c_str(), name2.c_str(), winner);
}

}}}

#endif
