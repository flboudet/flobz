#include "ios_udpmessagebox.h"
#include "ios_udpmessage.h"
#include <unistd.h>

namespace ios_fc {

class UdpPuncherClient : public MessageListener {
public:
    UdpPuncherClient(String hostName, int portNum);
    void punch(const String punchPoolName);
    void idle();
    void onMessage(Message &message);
private:
    UDPMessageBox *mbox;
};

UdpPuncherClient::UdpPuncherClient(String hostName, int portNum)
  : mbox(new UDPMessageBox(hostName, 0, portNum))
{
    mbox->addListener(this);
    //printf("GetSocketAddress(): %s\n", (const char *)(mbox->getSocketAddress().asString()));
    //printf("GetSocketPortNum(): %d\n", mbox->getDatagramSocket()->getSocketPortNum());
}

void UdpPuncherClient::punch(const String punchPoolName)
{
    Message *punchMsg = mbox->createMessage();
    Dirigeable *dirPunchMsg = dynamic_cast<Dirigeable *>(punchMsg);
    UDPPeerAddress localPeer(mbox->getSocketAddress(), mbox->getDatagramSocket()->getSocketPortNum());
    
    punchMsg->addString("PPOOL", punchPoolName);
    dirPunchMsg->addPeerAddress("LPEER", localPeer);
    punchMsg->addBoolProperty("RELIABLE", true);
    punchMsg->send();
    delete punchMsg;
}

void UdpPuncherClient::idle()
{
    mbox->idle();
}

void UdpPuncherClient::onMessage(Message &message)
{
    Dirigeable &dirMsg = dynamic_cast<Dirigeable &>(message);
    int type = message.getInt("TYPE");
    switch (type) {
        case 1:
            printf("Pool peer address\n");
            {
                UDPPeerAddress peerAddress = dirMsg.getPeerAddress("PEER");
                UDPPeerAddress localPeerAddress = dirMsg.getPeerAddress("LPEER");
                printf("Peer:  %s:%d\n", (const char *)(peerAddress.getSocketAddress().asString()), peerAddress.getPortNum());
                printf("LPeer: %s:%d\n", (const char *)(localPeerAddress.getSocketAddress().asString()), localPeerAddress.getPortNum());
                mbox->getDatagramSocket()->disconnect();
                for (int i = 0 ; i < 500 ; i++) {
                    Message *peerMsg = mbox->createMessage();
                    Dirigeable *dirPeerMsg = dynamic_cast<Dirigeable *>(peerMsg);
                    peerMsg->addString("CONNERIE", "Salut le monde!");
                    peerMsg->addInt("TYPE", 2);
                    dirPeerMsg->setPeerAddress(peerAddress);
                    peerMsg->send();
                    delete peerMsg;
                    mbox->idle();
                    usleep(10000);
                    Message *peerMsg2 = mbox->createMessage();
                    Dirigeable *dirPeerMsg2 = dynamic_cast<Dirigeable *>(peerMsg2);
                    peerMsg2->addString("CONNERIE", "Salut le monde local!");
                    peerMsg2->addInt("TYPE", 2);
                    dirPeerMsg2->setPeerAddress(peerAddress);
                    peerMsg2->send();
                    delete peerMsg2;
                    mbox->idle();
                    usleep(10000);
                }
            }
            break;
        case 2:
            printf("Message du peer par punch: %s\n", (const char *)(message.getString("CONNERIE")));
            break;
        default:
            printf("unknown\n");
            break;
    }
}

};

using namespace ios_fc;

int main(int argc, char *argv[])
{
    UdpPuncherClient toto(argv[1], 5432);;
    toto.punch(argv[2]);
    while (1) {
        sleep(1);
        toto.idle();
    }
    return 0;
}

