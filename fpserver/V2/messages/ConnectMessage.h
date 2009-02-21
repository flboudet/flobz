#ifndef CONNECT_MESSAGE_H
#define CONNECT_MESSAGE_H

namespace flobopuyo {
namespace server {
namespace v2 {

// Handles incoming connection requests
class ConnectMessage
{
private:
    ios_fc::Message *msg;
public:
    ConnectMessage(ios_fc::MessageBox &mbox, Peer *peer) {
        msg = mbox.createMessage();
        msg->addBoolProperty("RELIABLE", true);
        msg->addInt("CMD", PUYO_IGP_CONNECT);
        msg->addString("NAME", peer->name);
        msg->addInt("STATUS", peer->status);
        ios_fc::Dirigeable *dirNew = dynamic_cast<ios_fc::Dirigeable *>(msg);
        dirNew->addPeerAddress("ADDR", peer->addr);
    }

    ~ConnectMessage() {
        delete msg;
    }

    void sendTo(ios_fc::PeerAddress &addr) {
        ios_fc::Dirigeable *dirNew = dynamic_cast<ios_fc::Dirigeable *>(msg);
        dirNew->setPeerAddress(addr);
        msg->send();
    }
};

}}}

#endif


