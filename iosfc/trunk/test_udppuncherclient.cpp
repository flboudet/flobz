#include "ios_udpmessagebox.h"
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
}

void UdpPuncherClient::punch(const String punchPoolName)
{
    Message *punchMsg = mbox->createMessage();
    punchMsg->addString("PPOOL", punchPoolName);
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
	      for (int i = 0 ; i < 1000 ; i++) {
		Message *peerMsg = mbox->createMessage();
		Dirigeable *dirPeerMsg = dynamic_cast<Dirigeable *>(peerMsg);
		peerMsg->addString("CONNERIE", "Salut le monde!");
		peerMsg->addInt("TYPE", 2);
		dirPeerMsg->setPeerAddress(dirMsg.getPeerAddress("PEER"));
		peerMsg->send();
		delete peerMsg;
		mbox->idle();
		usleep(1000);
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

