#include <stdio.h>
#include "ios_datagramsocket.h"

int main()
{
    try {
        ios_fc::DatagramSocket toto;
        ios_fc::Datagram titi(ios_fc::SocketAddress("durandal"), 1478, ios_fc::VoidBuffer("Hello", 5), 5);
        toto.send(titi);
        ios_fc::Buffer<char> resultBuf(1024);
        ios_fc::Datagram result = toto.receive(resultBuf);
        printf("Recu:%s du port %d\n", (const void *)(result.getMessage()), result.getPortNum());
    } catch (ios_fc::Exception e) {
        e.printMessage();
    }
    return 0;
}
