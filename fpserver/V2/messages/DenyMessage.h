#ifndef DENY_MESSAGE_H
#define DENY_MESSAGE_H

namespace flobopuyo {
namespace server {
namespace v2 {

// Handles incoming connection requests
class DenyMessage
{
private:
    ios_fc::Message *msg;
public:
    DenyMessage(ios_fc::MessageBox &mbox, const ios_fc::String &errorString, const ios_fc::String &errorStringMore) {
        msg = mbox.createMessage();
        msg->addBoolProperty("RELIABLE", true);
        msg->addInt("CMD", PUYO_IGP_DENY);
        msg->addString("MSG", errorString);
        msg->addString("MSG_MORE", errorStringMore);
    }

    ~DenyMessage() {
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



