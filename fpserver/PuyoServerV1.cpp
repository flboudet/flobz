/* -*-mode:c; c-style:k&r; c-basic-offset:4; indent-tab-mode: nil; -*- */
#include "PuyoServerV1.h"

#include <stdio.h>
#include "ios_igpmessagebox.h"
#include "ios_memory.h"
#include "ios_dirigeable.h"
#include "ios_time.h"
#include "PuyoIgpDefs.h"

using namespace ios_fc;

namespace flobopuyo {
namespace server {

PuyoServerV1::PuyoServerV1(ios_fc::MessageBox &mbox)
 : mbox(mbox), timeMsBeforePeerTimeout(5000.), m_maxPeersAllowed(0)
{
}

bool PuyoServerV1::checkVersion(int clientVersion) const
{
    if (clientVersion == 1)
        return true;
    return false;
}

void PuyoServerV1::onMessage(Message &msg)
{
    try {
        // Gets the address of the sender of the message
        Dirigeable &dirMsg = dynamic_cast<Dirigeable &>(msg);
        PeerAddress address = dirMsg.getPeerAddress();
        // Gets the version of the FPIP protocol used by the client
        int protocolVersion = msg.getInt("V");

        switch (msg.getInt("CMD")) {
            // A client will send an alive message periodically to inform the server it is still connected
            case FLOBO_IGP_ALIVE:
            {
                GamePeer *currentPeer = getPeer(address);
                if (currentPeer != NULL)
                    updatePeer(currentPeer, msg.getInt("STATUS"));
                else
                    connectPeer(address, protocolVersion, msg.getString("NAME"), msg.getInt("STATUS"));
                break;
                }
            // A client will send a chat message to communicate with the other connected clients
            case FLOBO_IGP_CHAT:
                printf("Message de %s: %s\n", (const char *)msg.getString("NAME"), (const char *)msg.getString("MSG"));
            {
                Message *newMsg = mbox.createMessage();
                try {
                    newMsg->addBoolProperty("RELIABLE", true);
                    newMsg->addInt("CMD", FLOBO_IGP_CHAT);
                    newMsg->addString("NAME", msg.getString("NAME"));
                    newMsg->addString("MSG", msg.getString("MSG"));
                    Dirigeable *dirNew = dynamic_cast<Dirigeable *>(newMsg);
                    for (int i = 0, j = peers.size() ; i < j ; i++) {
                        printf("Diffusion au peer num %d\n", i);
                        dirNew->setPeerAddress(peers[i]->addr);
                        newMsg->send();
                    }
                } catch (Exception e) {}
                delete newMsg;
            }
                break;
            default:
                break;
        }
    } catch (Exception e) {}
}

void PuyoServerV1::idle()
{
    double time_ms = getTimeMs();
    for (int i = 0, j = peers.size() ; i < j ; i++) {
        GamePeer *currentPeer = peers[i];
        if ((time_ms - currentPeer->lastUpdate) > timeMsBeforePeerTimeout) {
            printf("PuyoServerV1 Peer timeout!\n");
            // Build disconnect message
            Message *newMsg = mbox.createMessage();
            newMsg->addBoolProperty("RELIABLE", true);
            newMsg->addInt("CMD", FLOBO_IGP_DISCONNECT);
            newMsg->addString("NAME", currentPeer->name);
            Dirigeable *dirNew = dynamic_cast<Dirigeable *>(newMsg);
            dirNew->addPeerAddress("ADDR", currentPeer->addr);
            // Delete peer
            peers.remove(currentPeer);
            delete currentPeer;
            j--;
            // Warn the others
            for (int i2 = 0 ; i2 < j ; i2++) {
                dirNew->setPeerAddress(peers[i2]->addr);
                newMsg->send();
            }
            delete newMsg;
        }
    }
}

PuyoServerV1::GamePeer * PuyoServerV1::getPeer(PeerAddress &addr) const
{
    for (int i = 0, j = peers.size() ; i < j ; i++) {
        if (peers[i]->addr == addr) {
            return peers[i];
        }
    }
    return NULL;
}

void PuyoServerV1::connectPeer(PeerAddress addr, int fpipVersion, const String name, int status)
{
    // First check wether the new peer can connect to the server or not
    bool accept = true;
    String denyErrorString;
    // Check if the client FPIP version is compatible with the server
    if (!checkVersion(fpipVersion)) {
        accept = false;
        denyErrorString = "Client version mismatch.";
    }
    // Check if we are exceeding the maximum number of allowed peers
    if ((m_maxPeersAllowed > 0) && (peers.size() == m_maxPeersAllowed)) {
        accept = false;
        denyErrorString = "Too many connected players";
    }

    if (accept) {
        // Send accept message
        Message *acceptMsg = mbox.createMessage();
        acceptMsg->addBoolProperty("RELIABLE", true);
        acceptMsg->addInt("CMD", FLOBO_IGP_ACCEPT);
        Dirigeable *dirAccept = dynamic_cast<Dirigeable *>(acceptMsg);
        dirAccept->setPeerAddress(addr);
        acceptMsg->send();
        delete acceptMsg;

        // Create the new peer
        GamePeer *newPeer = new GamePeer(addr, name);
        newPeer->lastUpdate = getTimeMs();
        printf("Nouveau peer: %s\n", (const char *)name);

        // Envoyer tous les peers connectes au petit nouveau
        for (int i = 0, j = peers.size() ; i < j ; i++) {
            Message *newMsg = mbox.createMessage();
            newMsg->addBoolProperty("RELIABLE", true);
            newMsg->addInt("CMD", FLOBO_IGP_CONNECT);
            newMsg->addString("NAME", peers[i]->name);
            newMsg->addInt("STATUS", peers[i]->status);
            Dirigeable *dirNew = dynamic_cast<Dirigeable *>(newMsg);
            dirNew->addPeerAddress("ADDR", peers[i]->addr);
            dirNew->setPeerAddress(addr);
            newMsg->send();
            delete newMsg;
        }
        // Inserer le petit nouveau a la liste
        peers.add(newPeer);

        // Envoyer l'info de connexion a tous les peers
        Message *newMsg = mbox.createMessage();
        newMsg->addBoolProperty("RELIABLE", true);
        newMsg->addInt("CMD", FLOBO_IGP_CONNECT);
        newMsg->addString("NAME", name);
        newMsg->addInt("STATUS", status);
        Dirigeable *dirNew = dynamic_cast<Dirigeable *>(newMsg);
        dirNew->addPeerAddress("ADDR", addr);
        for (int i = 0, j = peers.size() ; i < j ; i++) {
            printf("Diffusion connexion au peer num %d\n", i);
            dirNew->setPeerAddress(peers[i]->addr);
            newMsg->send();
        }
        delete newMsg;
    }
    else {
        // Send deny message
        Message *denyMsg = mbox.createMessage();
        denyMsg->addBoolProperty("RELIABLE", true);
        denyMsg->addInt("CMD", FLOBO_IGP_DENY);
        denyMsg->addString("MSG", denyErrorString);
        Dirigeable *dirDeny = dynamic_cast<Dirigeable *>(denyMsg);
        dirDeny->setPeerAddress(addr);
        denyMsg->send();
        delete denyMsg;
    }
}

void PuyoServerV1::updatePeer(GamePeer *peer, int status)
{
    peer->lastUpdate = getTimeMs();
    if (peer->status != status) {
        peer->status = status;
        // Send a STATUSCHANGE message to the other peers
        Message *newMsg = mbox.createMessage();
        newMsg->addBoolProperty("RELIABLE", true);
        newMsg->addInt("CMD", FLOBO_IGP_STATUSCHANGE);
        newMsg->addString("NAME", peer->name);
        newMsg->addInt("STATUS", status);
        Dirigeable *dirNew = dynamic_cast<Dirigeable *>(newMsg);
        dirNew->addPeerAddress("ADDR", peer->addr);
        for (int i = 0, j = peers.size() ; i < j ; i++) {
            printf("Diffusion connexion au peer num %d\n", i);
            dirNew->setPeerAddress(peers[i]->addr);
            newMsg->send();
        }
        delete newMsg;
    }
}

}}
