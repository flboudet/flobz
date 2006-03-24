#include "ios_udpmessagebox.h"
#include "ios_igpmessagebox.h"
#include "ios_udpmessage.h"
#include "PuyoIgpDefs.h"
#include <unistd.h>

using namespace ios_fc;

class UdpPuncherClient : public MessageListener {
public:
    UdpPuncherClient(String hostName, int portNum);
    void punch(const String punchPoolName);
    void idle();
    void onMessage(Message &message);
private:
    UDPMessageBox udpmbox;
    IgpMessageBox igpmbox;
};

UdpPuncherClient::UdpPuncherClient(String hostName, int portNum)
  : udpmbox(hostName, 0, portNum), igpmbox(udpmbox)
{
    igpmbox.addListener(this);
    //printf("GetSocketAddress(): %s\n", (const char *)(mbox->getSocketAddress().asString()));
    //printf("GetSocketPortNum(): %d\n", mbox->getDatagramSocket()->getSocketPortNum());
}

void UdpPuncherClient::punch(const String punchPoolName)
{
    int prevBound = igpmbox.getBound();
    igpmbox.bind(1);
    Message *punchMsg = igpmbox.createMessage();
    String localSocketAddress = udpmbox.getSocketAddress().asString();
    int localPortNum = udpmbox.getDatagramSocket()->getSocketPortNum();
    
    punchMsg->addInt("CMD", PUYO_IGP_NAT_TRAVERSAL);
    punchMsg->addString("PPOOL", punchPoolName);
    punchMsg->addString("LSOCKADDR", localSocketAddress);
    punchMsg->addInt("LPORTNUM", localPortNum);
    punchMsg->addBoolProperty("RELIABLE", true);
    punchMsg->send();
    delete punchMsg;
    igpmbox.bind(prevBound);
}

void UdpPuncherClient::idle()
{
    igpmbox.idle();
}

void UdpPuncherClient::onMessage(Message &message)
{
#ifdef AREFAIRE
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
                    dirPeerMsg2->setPeerAddress(localPeerAddress);
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
#endif
}


int main(int argc, char *argv[])
{
    UdpPuncherClient toto(argv[1], 4567);
    toto.punch(argv[2]);
    while (1) {
        sleep(1);
        toto.idle();
    }
    return 0;
}

