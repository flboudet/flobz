#include <stdio.h>
#include "ios_socket.h"
#include "ios_exception.h"
#include "ios_memory.h"
#include "ios_igpdatagram.h"

namespace ios_fc {

class IGPClient {
public:
    IGPClient(String hostName, int portID);
    void sendMessage(int igpID, VoidBuffer message);
    void idle();
private:
    Socket clientSocket;
};

IGPClient::IGPClient(String hostName, int portID) : clientSocket(hostName, portID)
{
    IGPDatagram::ClientMsgAutoAssignIDDatagram datagram;
    clientSocket.getOutputStream()->streamWrite(datagram.serialize());
}

void IGPClient::sendMessage(int igpID, VoidBuffer message)
{
    IGPDatagram::ClientMsgToClientDatagram datagram(igpID, message);
    clientSocket.getOutputStream()->streamWrite(datagram.serialize());
}

void IGPClient::idle()
{
    InputStream *input = clientSocket.getInputStream();
    int available = input->streamAvailable();
    if (available > 0) {
        VoidBuffer data(available);
        input->streamRead(data);
        IGPDatagram message(data);
        switch (message.getMsgIdent()) {
        case IGPDatagram::ServerMsgInformID: {
            IGPDatagram::ServerMsgInformIDDatagram informIDMessage(message);
            printf("Obtenu info sur id: %d\n", informIDMessage.getIgpIdent());
            break;
        }
        default:
            break;
        }
    }
}

};

#include <unistd.h>
int main()
{
    int count = 0;
    printf("Hello World!\n");
    try {
        //Socket::setFactory(&unixSocketFactory);
        ios_fc::IGPClient client("localhost", 4567);
	while (true) {
            client.idle();
            sleep(1);
            count++;
            if (count == 3) {
                client.sendMessage(55, ios_fc::VoidBuffer("hello", 5));
            }
        }
        //ios_fc::OutputStream *output = testSocket.getOutputStream();
        //output->streamWrite(ios_fc::VoidBuffer("Hello", 5));
    } catch (ios_fc::Exception e) {
        e.printMessage();
    }
    return 0;
}
