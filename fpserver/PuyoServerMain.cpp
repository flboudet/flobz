#include "ios_datagramsocket.h"
#include "ios_udpmessagebox.h"
#include "ios_igpmessagelistener.h"
#include "ios_igpvirtualpeermessagebox.h"
#include "PuyoServerIgpNatTraversal.h"
#include "PuyoServerV1.h"
#include "PuyoServerV2.h"
#include <stdlib.h>

using namespace ios_fc;
using namespace flobopuyo::server;

#define VERSION_1

int main(int argc, const char * argv[])
{
    int port = 4567;  // Default port number
    int protocol = 2; // Default protocol number
    if (argc >= 2) port = atoi(argv[1]);
    if (argc >= 3) protocol = atoi(argv[2]);
    fprintf(stderr, "Usage %s [port number] [protocol number]\n", argv[0]);
    fprintf(stderr, "Starting server at port %d, using protocol %d\n", port, protocol);
  
    Selector serverSelector;
    DatagramSocket serverSocket(port);
    UDPMessageBox messageBox(&serverSocket);
    IgpMessageListener listener(messageBox);
    
    messageBox.addListener(&listener);
    messageBox.addSessionListener(&listener);
    serverSelector.addSelectable(&serverSocket);
    
    IgpVirtualPeerMessageBox igpMBox(listener, 1);
    PuyoIgpNatTraversal natPuncher(igpMBox, listener);
    igpMBox.addListener(&natPuncher);
    
    PuyoServer *responder;
    switch (protocol) {
        case 1: responder = new PuyoServerV1(igpMBox); break;
        case 2: responder = new PuyoServerV2(igpMBox); break;
        default:
                fprintf(stderr, "Erros: valid protocols are 1 and 2");
    }
    igpMBox.addListener(responder);

    try {
        while (true) {
            serverSelector.select(10);
            try {
                messageBox.idle();
                // Pb de timeout par la
                responder->idle();
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

