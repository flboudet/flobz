#include "ios_selector.h"
#include "ios_datagramsocket.h"
#include "ios_udpmessagebox.h"
#include "ios_igpdatagram.h"
#include "ios_dirigeable.h"
#include <stdio.h>

namespace ios_fc {

class IgpMessageListener : public MessageListener {
public:
    IgpMessageListener(MessageBox &mbox) : mbox(mbox), currentAutoIgpIdent(firstAutoIgpIdent) {}
    void onMessage(Message &message);
    int getUniqueIGPId();
    bool igpIdValidAndUnique(int igpIdent);
    class PeerRecord;
    PeerRecord *getPeer(int igpIdent) const;
    Message *createMessage() { return mbox.createMessage(); }
private:
    static const int firstAutoIgpIdent;
    MessageBox &mbox;
    int currentAutoIgpIdent;
    PeerRecord *findPeer(PeerAddress address);
    AdvancedBuffer<PeerRecord *> knownPeers;
};

const int IgpMessageListener::firstAutoIgpIdent = 32768;

class IgpMessageListener::PeerRecord {
public:
    PeerRecord(PeerAddress address, IgpMessageListener *pool) : address(address), pool(pool), valid(false) {}
    inline PeerAddress getAddress() const { return address; }
    void datagramReceived(IGPDatagram &message);
    inline int getIgpIdent() const { return igpID; }
private:
    void sendIGPIdent();
    PeerAddress address;
    IgpMessageListener *pool;
    int igpID;
    bool valid;
};

void IgpMessageListener::PeerRecord::sendIGPIdent()
{
    IGPDatagram::ServerMsgInformIDDatagram reply(pool->createMessage(), igpID);
    Dirigeable *msg = dynamic_cast<Dirigeable *>(reply.getMessage());
    if (msg != NULL) {
        //printf("setpeeraddress\n");
        msg->setPeerAddress(this->address);
        //printf("setpeeraddress done\n");
        reply.getMessage()->send();
        //printf("envoye!\n");
    }
    else {
        printf("msg NULL!\n");
    }
}

void IgpMessageListener::PeerRecord::datagramReceived(IGPDatagram &message)
{
    switch (message.getMsgIdent()) {
    case IGPDatagram::ClientMsgAutoAssignID:
        igpID = pool->getUniqueIGPId();
        printf("Auto-assign ID:%d\n", igpID);
        valid=true;
        sendIGPIdent();
        break;
    case IGPDatagram::ClientMsgAssignID: {
        IGPDatagram::ClientMsgAssignIDDatagram msgReceived(message);
        printf("Assign ID\n");
        if (pool->igpIdValidAndUnique(msgReceived.getIgpIdent())) {
            valid=true;
            igpID = msgReceived.getIgpIdent();
            sendIGPIdent();
        }
        else {
            printf("Adresse igp invalide:%d\n", msgReceived.getIgpIdent());
        }
        break;
    }
    case IGPDatagram::ClientMsgGetID:
        sendIGPIdent();
        break;
    case IGPDatagram::ClientMsgToClient: {
        IGPDatagram::ClientMsgToClientDatagram msgReceived(message);
        bool reliable = false;
        Message *rawmsg = msgReceived.getMessage();
        if (rawmsg->hasBoolProperty("RELIABLE"))
            reliable = rawmsg->getBoolProperty("RELIABLE");
        IGPDatagram::ServerMsgToClientDatagram msgToSend(pool->createMessage(),
                                                        igpID, msgReceived.getIgpIdent(),
                                                        msgReceived.getIgpMessage(),
                                                        reliable);
        PeerRecord *destPeer = pool->getPeer(msgReceived.getIgpIdent());
        if (destPeer != NULL) {
            Dirigeable *msg = dynamic_cast<Dirigeable *>(msgToSend.getMessage());
            if (msg != NULL) {
                msg->setPeerAddress(destPeer->getAddress());
                msgToSend.getMessage()->send();
            }
        }
        else {
            printf("DEST not found!!!\n");
        }
        break;
    }
    default:
        break;
	}
}

IgpMessageListener::PeerRecord *IgpMessageListener::findPeer(PeerAddress address)
{
    for (int i = 0, j = knownPeers.size() ; i < j ; i++) {
        if (knownPeers[i]->getAddress() == address) {
            //printf("Trouve !\n");
            return knownPeers[i];
        }
    }
    return NULL;
}

void IgpMessageListener::onMessage(Message &data)
{
    //printf("Cool, un nouveau message!\n");
    Dirigeable &dir = dynamic_cast<Dirigeable &>(data);
    
    PeerAddress msgAddress = dir.getPeerAddress();
    PeerRecord *currentPeer = findPeer(msgAddress);
    if (currentPeer == NULL) {
        currentPeer = new PeerRecord(msgAddress, this);
        knownPeers.add(currentPeer);
        //printf("Nouveau peer !\n");
    }
    
    IGPDatagram message(&data);
    currentPeer->datagramReceived(message);
}

int IgpMessageListener::getUniqueIGPId()
{
    return currentAutoIgpIdent++;
}

bool IgpMessageListener::igpIdValidAndUnique(int igpIdent)
{
    if ((igpIdent <= 0) || (igpIdent >= firstAutoIgpIdent))
        return false;
    for (int i = 0, j = knownPeers.size() ; i < j ; i++) {
        if (igpIdent == knownPeers[i]->getIgpIdent())
            return false;
    }
    return true;
}

IgpMessageListener::PeerRecord *IgpMessageListener::getPeer(int igpIdent) const
{
    for (int i = 0, j = knownPeers.size() ; i < j ; i++) {
        if (igpIdent == knownPeers[i]->getIgpIdent())
            return knownPeers[i];
    }
    return NULL;
}

}

using namespace ios_fc;

int main()
{
    Selector serverSelector;
    DatagramSocket serverSocket(4567);
    UDPMessageBox messageBox(&serverSocket);
    IgpMessageListener listener(messageBox);
    
    messageBox.addListener(&listener);
    serverSelector.addSelectable(&serverSocket);
    
    try {
        while (true) {
            serverSelector.select();
            messageBox.idle();
        }
    }
    catch (ios_fc::Exception e) {
        e.printMessage();
    }
    return 0;
}
