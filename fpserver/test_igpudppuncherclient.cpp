#include "ios_udpmessagebox.h"
#include "ios_igpmessagebox.h"
#include "ios_udpmessage.h"
#include "ios_time.h"
#include "PuyoIgpDefs.h"
#include <unistd.h>

using namespace ios_fc;

class UdpPuncherClient : public MessageListener {
public:
    UdpPuncherClient(String hostName, int portNum);
    void punch(const String punchPoolName);
    void idle();
    void onMessage(Message &message);
    void sendGarbageMessage();
private:
    UDPMessageBox udpmbox;
    IgpMessageBox *igpmbox;
    String peerAddressString, peerLocalAddressString;
    int peerPortNum, peerLocalPortNum;
    enum {
        TRY_NONE = 0,
        TRY_PUBLICADDR = 1,
        TRY_PUBLICADDR_NEXTPORT = 2,
        TRY_LOCALADDR = 3,
        FAILED = 4,
        SUCCESS = 5
    };
    int currentStrategy;
    double strategyTimeout, timeToNextStrategy;
};

UdpPuncherClient::UdpPuncherClient(String hostName, int portNum)
  : udpmbox(hostName, 0, portNum), igpmbox(new IgpMessageBox(udpmbox)), currentStrategy(TRY_NONE), strategyTimeout(2000.)
{
    igpmbox->addListener(this);
    //printf("GetSocketAddress(): %s\n", (const char *)(mbox->getSocketAddress().asString()));
    //printf("GetSocketPortNum(): %d\n", mbox->getDatagramSocket()->getSocketPortNum());
}

void UdpPuncherClient::punch(const String punchPoolName)
{
    int prevBound = igpmbox->getBound();
    igpmbox->bind(1);
    Message *punchMsg = igpmbox->createMessage();
    String localSocketAddress = udpmbox.getSocketAddress().asString();
    int localPortNum = udpmbox.getDatagramSocket()->getSocketPortNum();
    
    punchMsg->addInt("CMD", PUYO_IGP_NAT_TRAVERSAL);
    punchMsg->addString("PPOOL", punchPoolName);
    punchMsg->addString("LSOCKADDR", localSocketAddress);
    punchMsg->addInt("LPORTNUM", localPortNum);
    punchMsg->addBoolProperty("RELIABLE", true);
    punchMsg->send();
    delete punchMsg;
    igpmbox->bind(prevBound);
}

void UdpPuncherClient::idle()
{
    if (igpmbox != NULL)
        igpmbox->idle();
    else {
        double currentTime = getTimeMs();
        switch (currentStrategy) {
            case TRY_NONE: {
                // Switch to TRY_PUBLICADDR
                printf("Trying the TRY_PUBLICADDR strategy\n");
                SocketAddress sockPubAddr(peerAddressString);
                udpmbox.addListener(this);
                udpmbox.getDatagramSocket()->connect(sockPubAddr, peerPortNum);
                PeerAddress peerPubAddr = udpmbox.createPeerAddress(sockPubAddr, peerPortNum);
                udpmbox.bind(peerPubAddr);
                currentStrategy = TRY_PUBLICADDR;
                timeToNextStrategy = currentTime + strategyTimeout;
                break;
            }
            case TRY_PUBLICADDR:
                if (timeToNextStrategy < currentTime) {
                    // Switch to TRY_PUBLICADDR_NEXTPORT
                    printf("Trying the TRY_PUBLICADDR_NEXTPORT strategy\n");
                    SocketAddress sockPubAddr(peerAddressString);
                    udpmbox.getDatagramSocket()->connect(sockPubAddr, peerPortNum + 1);
                    PeerAddress peerPubAddr = udpmbox.createPeerAddress(sockPubAddr, peerPortNum + 1);
                    udpmbox.bind(peerPubAddr);
                    currentStrategy = TRY_PUBLICADDR_NEXTPORT;
                    timeToNextStrategy = currentTime + strategyTimeout;
                }
                break;
            case TRY_PUBLICADDR_NEXTPORT:
                if (timeToNextStrategy < currentTime) {
                    // Switch to TRY_LOCALADDR
                    printf("Trying the TRY_LOCALADDR strategy\n");
                    SocketAddress sockLocalAddr(peerLocalAddressString);
                    udpmbox.getDatagramSocket()->connect(sockLocalAddr, peerLocalPortNum);
                    PeerAddress peerLocalAddr = udpmbox.createPeerAddress(sockLocalAddr, peerLocalPortNum);
                    udpmbox.bind(peerLocalAddr);
                    currentStrategy = TRY_LOCALADDR;
                    timeToNextStrategy = currentTime + strategyTimeout;
                }
                break;
            case TRY_LOCALADDR:
                if (timeToNextStrategy < currentTime) {
                    // Switch to FAILED
                    printf("Failed to establish NAT traversal\n");
                    currentStrategy = FAILED;
                }
                break;
            default:
                break;
        }
        sendGarbageMessage();
        udpmbox.idle();
    }
}

void UdpPuncherClient::onMessage(Message &msg)
{
    switch (msg.getInt("CMD")) {
        case PUYO_IGP_NAT_TRAVERSAL: {
            peerAddressString = msg.getString("SOCKADDR");
            peerLocalAddressString = msg.getString("LSOCKADDR");
            peerPortNum = msg.getInt("PORTNUM");
            peerLocalPortNum = msg.getInt("LPORTNUM");
            printf("Peer:    %s:%d\n", (const char *)peerAddressString, peerPortNum);
            printf("Peer(L): %s:%d\n", (const char *)peerLocalAddressString, peerLocalPortNum);
            
            // Destroy the igp messagebox
            delete igpmbox;
            igpmbox = NULL;
            udpmbox.getDatagramSocket()->disconnect();
            
            // Connect the udp messagebox to the local address of the peer
            /*SocketAddress sockLocAddr(peerLocalAddressString);
            udpmbox.addListener(this);
            udpmbox.getDatagramSocket()->connect(sockLocAddr, peerPortNum);
            PeerAddress peerAddr = udpmbox.createPeerAddress(sockLocAddr, peerPortNum);
            udpmbox.bind(peerAddr);*/
            break;
        }
        case PUYO_IGP_NAT_TRAVERSAL_GARBAGE:
            printf("Garbage msg received: %s\n", (const char *)(msg.getString("GARBAGE")));
            currentStrategy = SUCCESS;
            break;
        default:
            break;
    }
}

void UdpPuncherClient::sendGarbageMessage()
{
    //printf("Envoi garbage\n");
    Message *garbMsg = udpmbox.createMessage();
    
    garbMsg->addInt("CMD", PUYO_IGP_NAT_TRAVERSAL_GARBAGE);
    garbMsg->addString("GARBAGE", "Connerie");
    
    garbMsg->send();
    delete garbMsg;
}

int main(int argc, char *argv[])
{
    UdpPuncherClient toto(argv[1], 4567);
    toto.punch(argv[2]);
    while (1) {
        usleep(20000);
        toto.idle();
    }
    return 0;
}

