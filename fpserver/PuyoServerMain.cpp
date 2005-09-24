#include "ios_datagramsocket.h"
#include "ios_udpmessagebox.h"
#include "ios_igpmessagelistener.h"
#include "ios_igpvirtualpeermessagebox.h"
#include "PuyoServerIgpResponder.h"

using namespace ios_fc;

int main()
{
    Selector serverSelector;
    DatagramSocket serverSocket(4567);
    UDPMessageBox messageBox(&serverSocket);
    IgpMessageListener listener(messageBox);
    
    messageBox.addListener(&listener);
    messageBox.addSessionListener(&listener);
    serverSelector.addSelectable(&serverSocket);
    
    IgpVirtualPeerMessageBox igpMBox(listener, 1);
    PuyoIgpResponder responder(igpMBox);
    igpMBox.addListener(&responder);
    
    try {
        while (true) {
            serverSelector.select(10);
            try {
                messageBox.idle();
                // Pb de timeout par la
                responder.idle();
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

