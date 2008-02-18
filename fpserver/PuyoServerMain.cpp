#include "ios_datagramsocket.h"
#include "ios_udpmessagebox.h"
#include "ios_igpmessagelistener.h"
#include "ios_igpvirtualpeermessagebox.h"
#include "PuyoServerIgpResponder.h"
#include "PuyoServerIgpNatTraversal.h"
#include <stdlib.h>

using namespace ios_fc;

int main(int argc, const char * argv[])
{
  int port;
    if (argc >= 2)  port = atoi(argv[1]);
    else port = 4567;
    fprintf (stderr, "Starting server at port %d\n", port);
  
    Selector serverSelector;
    DatagramSocket serverSocket(port);
    UDPMessageBox messageBox(&serverSocket);
    IgpMessageListener listener(messageBox);
    
    messageBox.addListener(&listener);
    messageBox.addSessionListener(&listener);
    serverSelector.addSelectable(&serverSocket);
    
    IgpVirtualPeerMessageBox igpMBox(listener, 1);
    PuyoIgpResponder responder(igpMBox);
    PuyoIgpNatTraversal natPuncher(igpMBox, listener);
    igpMBox.addListener(&responder);
    igpMBox.addListener(&natPuncher);
    
    try {
        while (true) {
            serverSelector.select(10);
            try {
                messageBox.idle();
                // Pb de timeout par la
                responder.idle();
                natPuncher.idle();
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

