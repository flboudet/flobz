#include "ios_selector.h"
#include "ios_datagramsocket.h"
#include "ios_udpmessagebox.h"
#include "ios_dirigeable.h"
#include <stdio.h>
#include <unistd.h>

namespace ios_fc {

class UdpPuncher : public MessageListener {
public:
    UdpPuncher(MessageBox *mbox) : mbox(mbox) {}
    void onMessage(Message &message);
private:
    class PunchPool;
    AdvancedBuffer<PunchPool *> pools;
    MessageBox *mbox;
};

class UdpPuncher::PunchPool {
public:
    PunchPool(const String punchPoolName, const PeerAddress creatorAddress, const PeerAddress creatorLocalAddress)
        : punchPoolName(punchPoolName), creatorAddress(creatorAddress), creatorLocalAddress(creatorLocalAddress) {}
    inline const String getPunchPoolName() const { return punchPoolName; }
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

void UdpPuncher::onMessage(Message &message)
{
    Dirigeable &dirMsg = dynamic_cast<Dirigeable &>(message);
    const String punchPoolName = message.getString("PPOOL");
    printf("Message pool %s!\n", (const char *)punchPoolName);
    
    // Search if the pool exists
    for (int i = 0 ; i < pools.size() ; i++) {
        if (pools[i]->getPunchPoolName() == punchPoolName) {
            printf("Pool found!\n");
            pools[i]->dispatchInformations(dirMsg.getPeerAddress(), dirMsg.getPeerAddress("LPEER"), mbox);
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
                messageBox.idle();
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

