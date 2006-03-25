/* -*-mode:c; c-style:k&r; c-basic-offset:4; indent-tab-mode: nil; -*- */
#include "ios_igpmessage.h"
#include "ios_udpmessage.h"
#include "PuyoServerIgpNatTraversal.h"
#include "PuyoIgpDefs.h"
#include "ios_time.h"

class PuyoIgpNatTraversal::PunchPool {
public:
    PunchPool(const String punchPoolName, String creatorAddress, int creatorPortNum, String creatorLocalAddress, int creatorLocalPortNum, PeerAddress creatorPeerAddress)
        : lastUpdate(getTimeMs()), punchPoolName(punchPoolName),
          creatorAddress(creatorAddress), creatorPort(creatorPortNum), creatorLocalAddress(creatorLocalAddress), creatorLocalPort(creatorLocalPortNum), creatorPeerAddress(creatorPeerAddress) {}
    inline const String getPunchPoolName() const { return punchPoolName; }
    void dispatchInformations(String guestAddress, int guestPortNum, String guestLocalAddress, int guestLocalPortNum, PeerAddress guestPeerAddress, MessageBox *mbox);
    double lastUpdate;
private:
    String punchPoolName;
    String creatorAddress, creatorLocalAddress;
    PeerAddress creatorPeerAddress;
    int creatorPort, creatorLocalPort;
};

void PuyoIgpNatTraversal::PunchPool::dispatchInformations(String guestAddress, int guestPortNum, String guestLocalAddress, int guestLocalPortNum, PeerAddress guestPeerAddress, MessageBox *mbox)
{
    // Message for the pool creator
    Message *peerAMsg = mbox->createMessage();
    Dirigeable *dirAMsg = dynamic_cast<Dirigeable *>(peerAMsg);
    peerAMsg->addInt("CMD", PUYO_IGP_NAT_TRAVERSAL);
    peerAMsg->addString("PPOOL", punchPoolName);
    peerAMsg->addBoolProperty("RELIABLE", true);
    peerAMsg->addString("SOCKADDR", guestAddress);
    peerAMsg->addInt("PORTNUM", guestPortNum);
    peerAMsg->addString("LSOCKADDR", guestLocalAddress);
    peerAMsg->addInt("LPORTNUM", guestLocalPortNum);
    dirAMsg->setPeerAddress(creatorPeerAddress);
    peerAMsg->send();
    delete peerAMsg;
    
    // Message for the pool guest
    Message *peerBMsg = mbox->createMessage();
    Dirigeable *dirBMsg = dynamic_cast<Dirigeable *>(peerAMsg);
    peerBMsg->addInt("CMD", PUYO_IGP_NAT_TRAVERSAL);
    peerBMsg->addString("PPOOL", punchPoolName);
    peerBMsg->addBoolProperty("RELIABLE", true);
    peerBMsg->addString("SOCKADDR", creatorAddress);
    peerBMsg->addInt("PORTNUM", creatorPort);
    peerBMsg->addString("LSOCKADDR", creatorLocalAddress);
    peerBMsg->addInt("LPORTNUM", creatorLocalPort);
    dirBMsg->setPeerAddress(guestPeerAddress);
    peerBMsg->send();
    delete peerBMsg;
}

void PuyoIgpNatTraversal::onMessage(Message &msg)
{
    //printf("Message recu!\n");
    switch (msg.getInt("CMD")) {
        case PUYO_IGP_NAT_TRAVERSAL: {
            Dirigeable &dirMsg = dynamic_cast<Dirigeable &>(msg);
            PeerAddress address = dirMsg.getPeerAddress();
            IgpMessage::IgpPeerAddressImpl *impl =
                static_cast<IgpMessage::IgpPeerAddressImpl *>(address.getImpl());
            printf("Demande de peeraddress udp du peer igp %d\n", impl->getIgpIdent());
            UDPPeerAddress udpAddress = igpListener.getPeerAddress(impl->getIgpIdent());
            printf("Peer:  %s:%d\n", (const char *)(udpAddress.getSocketAddress().asString()), udpAddress.getPortNum());
            printf("Local address:  %s:%d\n", (const char *)(msg.getString("LSOCKADDR")), msg.getInt("LPORTNUM"));
            String poolName = msg.getString("PPOOL");
            
            for (int i = 0, j = pools.size() ; i < j ; i++) {
                PunchPool *currentPool = pools[i];
                if (currentPool->getPunchPoolName() == poolName) {
                    currentPool->dispatchInformations(udpAddress.getSocketAddress().asString(), udpAddress.getPortNum(),
                                                msg.getString("LSOCKADDR"), msg.getInt("LPORTNUM"), address, &mbox);
                    pools.remove(currentPool);
                    delete currentPool;
                    return;
                }
            }
            pools.add(new PunchPool(poolName,
                                    udpAddress.getSocketAddress().asString(), udpAddress.getPortNum(),
                                    msg.getString("LSOCKADDR"), msg.getInt("LPORTNUM"), address));
            break;
        }
        default:
            break;
    }
}

void PuyoIgpNatTraversal::idle()
{
    int nbPunch = pools.size();
    if (nbPunch > 0) {
        double time_ms = getTimeMs();
        for (int i = 0 ; i < nbPunch ; i++) {
            PunchPool *currentPool = pools[i];
            if ((time_ms - currentPool->lastUpdate) > timeMsBeforePunchTimeout) {
                printf("Pool timeout!\n");
                // Delete pool
                pools.remove(currentPool);
                delete currentPool;
                nbPunch--;
            }
        }
    }
}
