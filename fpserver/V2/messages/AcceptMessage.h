#ifndef ACCEPT_MESSAGE_H
#define ACCEPT_MESSAGE_H

namespace flobopuyo {
namespace server {
namespace v2 {

// Handles incoming connection requests
class AcceptMessage
{
private:
    ios_fc::Message *msg;
public:
    AcceptMessage(ios_fc::MessageBox &mbox) {
        msg = mbox.createMessage();
        msg->addBoolProperty("RELIABLE", true);
        msg->addInt("CMD", FLOBO_IGP_ACCEPT);
    }

    ~AcceptMessage() {
        delete msg;
    }

    void sendTo(ios_fc::PeerAddress &addr) {
        ios_fc::Dirigeable *dirAccept = dynamic_cast<ios_fc::Dirigeable *>(msg);
        dirAccept->setPeerAddress(addr);
        msg->send();
    }
};

}}}

#endif

