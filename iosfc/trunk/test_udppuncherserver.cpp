#include "ios_selector.h"
#include "ios_datagramsocket.h"
#include "ios_udpmessagebox.h"
#include "ios_dirigeable.h"
#include <stdio.h>
#include <unistd.h>

#define SERVERCYCLEMS 10
#define POOLTTLMS 10000
#define POOLTTLTICKS POOLTTLMS/SERVERCYCLEMS

namespace ios_fc {

class UdpPuncher : public MessageListener {
public:
    UdpPuncher(MessageBox *mbox) : mbox(mbox) {}
    void idle();
    void onMessage(Message &message);
private:
    class PunchPool;
    AdvancedBuffer<PunchPool *> pools;
    MessageBox *mbox;
};

class UdpPuncher::PunchPool {
public:
    PunchPool(const String punchPoolName, const PeerAddress creatorAddress, const PeerAddress creatorLocalAddress)
        : punchPoolName(punchPoolName), timeToLive(POOLTTLTICKS),
          creatorAddress(creatorAddress), creatorLocalAddress(creatorLocalAddress) {}
    inline const String getPunchPoolName() const { return punchPoolName; }
    int timeToLive;
    void dispatchInformations(const PeerAddress guestAddress, const PeerAddress guestLocalAddress, MessageBox *mbox);
private:
    const String punchPoolName;
    const PeerAddress creatorAddress, creatorLocalAddress;
};

void UdpPuncher::PunchPool::dispatchInformations(const PeerAddress guestAddress, const PeerAddress guestLocalAddress, MessageBox *mbox)
{
    // Message for the pool creator
    Message *peerAMsg = mbox->createMessage();
    Dirigeable *dirAMsg = dynamic_cast<Dirigeable *>(peerAMsg);
    peerAMsg->addInt("TYPE", 1);
    peerAMsg->addString("PPOOL", punchPoolName);
    peerAMsg->addBoolProperty("RELIABLE", true);
    dirAMsg->addPeerAddress("PEER", guestAddress);
    dirAMsg->addPeerAddress("LPEER", guestLocalAddress);
    dirAMsg->setPeerAddress(creatorAddress);
    peerAMsg->send();
    delete peerAMsg;
    
    // Message for the pool guest
    Message *peerBMsg = mbox->createMessage();
    Dirigeable *dirBMsg = dynamic_cast<Dirigeable *>(peerBMsg);
    peerAMsg->addInt("TYPE", 1);
    peerBMsg->addString("PPOOL", punchPoolName);
    peerBMsg->addBoolProperty("RELIABLE", true);
    dirBMsg->addPeerAddress("PEER", creatorAddress);
    dirBMsg->addPeerAddress("LPEER", creatorLocalAddress);
    dirBMsg->setPeerAddress(guestAddress);
    peerBMsg->send();
    delete peerBMsg;
}

void UdpPuncher::idle()
{
    mbox->idle();
    // handle abandonned pools deletion
    for (int i = pools.size()-1 ; i >= 0 ; i--) {
        PunchPool *currentPool = pools[i];
        currentPool->timeToLive--;
        if (currentPool->timeToLive <= 0) {
            printf("pool %s: TTL expired!\n", (const char *)(currentPool->getPunchPoolName()));
            pools.remove(currentPool);
            delete currentPool;
        }
    }
}

void UdpPuncher::onMessage(Message &message)
{
    Dirigeable &dirMsg = dynamic_cast<Dirigeable &>(message);
    const String punchPoolName = message.getString("PPOOL");
    printf("Message pool %s!\n", (const char *)punchPoolName);
    
    // Search if the pool exists
    for (int i = pools.size()-1 ; i >= 0 ; i--) {
        if (pools[i]->getPunchPoolName() == punchPoolName) {
            PunchPool *currentPool = pools[i];
            printf("Pool found!\n");
            currentPool->dispatchInformations(dirMsg.getPeerAddress(), dirMsg.getPeerAddress("LPEER"), mbox);
            pools.remove(currentPool);
            delete currentPool;
            return;
        }
    }
    // Else let's create the pool
    pools.add(new PunchPool(punchPoolName, dirMsg.getPeerAddress(), dirMsg.getPeerAddress("LPEER")));
}

};

using namespace ios_fc;

int main()
{
    Selector serverSelector;
    DatagramSocket serverSocket(5432);
    UDPMessageBox messageBox(&serverSocket);
    UdpPuncher puncher(&messageBox);
    
    messageBox.addListener(&puncher);
    serverSelector.addSelectable(&serverSocket);
    
    try {
        while (true) {
            serverSelector.select(10);
            try {
                puncher.idle();
            }
            catch (ios_fc::Exception e) {
                e.printMessage();
            }
        }
    }
    catch (ios_fc::Exception e) {
        e.printMessage();
    }
    return 0;
}

