#include <stdio.h>
#include "ios_igpmessagebox.h"
#include "ios_memory.h"
#include "ios_dirigeable.h"
#include "ios_time.h"
#include "PuyoIgpDefs.h"
#include "PuyoServerIgpResponder.h"

using namespace ios_fc;

void PuyoIgpResponder::onMessage(Message &msg)
{
    Dirigeable &dirMsg = dynamic_cast<Dirigeable &>(msg);
    PeerAddress address = dirMsg.getPeerAddress();
    //printf("Message recu!\n");
    switch (msg.getInt("CMD")) {
        case PUYO_IGP_ALIVE:
            //printf("Message d'identification\n");
            updatePeer(address, msg.getString("NAME"));
            break;
        case PUYO_IGP_CHAT:
            printf("Message de %s: %s\n", (const char *)msg.getString("NAME"), (const char *)msg.getString("MSG"));
            {
                Message *newMsg = mbox.createMessage();
                newMsg->addBoolProperty("RELIABLE", true);
                newMsg->addInt("CMD", PUYO_IGP_CHAT);
                newMsg->addString("NAME", msg.getString("NAME"));
                newMsg->addString("MSG", msg.getString("MSG"));
                Dirigeable *dirNew = dynamic_cast<Dirigeable *>(newMsg);
                for (int i = 0, j = peers.size() ; i < j ; i++) {
                    printf("Diffusion au peer num %d\n", i);
                    dirNew->setPeerAddress(peers[i]->addr);
                    newMsg->send();
                }
                delete newMsg;
            }
            break;
        default:
            break;
    }
}
        
void PuyoIgpResponder::idle()
{
    double time_ms = getTimeMs();
    for (int i = 0, j = peers.size() ; i < j ; i++) {
        GamePeer *currentPeer = peers[i];
        if ((time_ms - currentPeer->lastUpdate) > timeMsBeforePeerTimeout) {
            printf("PuyoIgpResponder Peer timeout!\n");
            // Build disconnect message
            Message *newMsg = mbox.createMessage();
            newMsg->addBoolProperty("RELIABLE", true);
            newMsg->addInt("CMD", PUYO_IGP_DISCONNECT);
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
        }
    }
}

void PuyoIgpResponder::updatePeer(PeerAddress addr, const String name)
{
    GamePeer *currentPeer = NULL;
    for (int i = 0, j = peers.size() ; i < j ; i++) {
        if (peers[i]->addr == addr) {
            currentPeer = peers[i];
            currentPeer->lastUpdate = getTimeMs();
        }
    }
    if (currentPeer == NULL) {
        // Verifier l'autorisation de connexion ici
        GamePeer *newPeer = new GamePeer(addr, name);
	newPeer->lastUpdate = getTimeMs();
        printf("Nouveau peer: %s\n", (const char *)name);
        
        // Envoyer tous les peers connectes au petit nouveau
        for (int i = 0, j = peers.size() ; i < j ; i++) {
            Message *newMsg = mbox.createMessage();
            newMsg->addBoolProperty("RELIABLE", true);
            newMsg->addInt("CMD", PUYO_IGP_CONNECT);
            newMsg->addString("NAME", peers[i]->name);
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
        newMsg->addInt("CMD", PUYO_IGP_CONNECT);
        newMsg->addString("NAME", name);
        Dirigeable *dirNew = dynamic_cast<Dirigeable *>(newMsg);
        dirNew->addPeerAddress("ADDR", addr);
        for (int i = 0, j = peers.size() ; i < j ; i++) {
            printf("Diffusion connexion au peer num %d\n", i);
            dirNew->setPeerAddress(peers[i]->addr);
            newMsg->send();
        }
        delete newMsg;
    }
}
