#include "ios_selector.h"
#include "ios_datagramsocket.h"
#include "ios_udpmessagebox.h"
#include <stdio.h>

namespace ios_fc {

class IgpMessageListener : public MessageListener {
public:
    void onMessage(Message &message);
};

void IgpMessageListener::onMessage(Message &data)
{
    printf("Cool, un nouveau message!\n");
    IGPDatagram message(data);
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
        IGPDatagram::ServerMsgToClientDatagram msgToSend(igpID, msgReceived.getIgpIdent(), msgReceived.getMessage());
        IGPServerConnection *destConnection = pool->getConnection(msgReceived.getIgpIdent());
        if (destConnection != NULL) {
            destConnection->clientSocket->getOutputStream()->streamWrite(msgToSend.serialize());
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

}

using namespace ios_fc;

int main()
{
    Selector serverSelector;
    DatagramSocket serverSocket(4567);
    UDPMessageBox messageBox(&serverSocket);
    IgpMessageListener listener;
    
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
