#include <stdio.h>
#include "ios_datagramsocket.h"
#include "ios_selector.h"

int main()
{
    try {
        ios_fc::DatagramSocket toto;
        ios_fc::Datagram titi(ios_fc::SocketAddress("durandal"), 1478, ios_fc::VoidBuffer("Hello", 5), 5);
        toto.send(titi);
        ios_fc::Selector test;
        test.addSelectable(&toto);
        test.select();
        ios_fc::Buffer<char> resultBuf(1024);
        ios_fc::Datagram result = toto.receive(resultBuf);
        printf("Recu:%s du port %d\n", (const void *)(result.getMessage()), result.getPortNum());
        ios_fc::Datagram titi2(result.getAddress(), result.getPortNum(), ios_fc::VoidBuffer("Tiens", 5), 5);
        toto.send(titi2);
    } catch (ios_fc::Exception e) {
        e.printMessage();
    }
    return 0;
}
