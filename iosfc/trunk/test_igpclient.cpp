#include <stdio.h>
#include "ios_igpclient.h"

#include <unistd.h>

class MyListener : public ios_fc::IGPClientMessageListener {
public:
    void onMessage(ios_fc::VoidBuffer message, int igpOriginIdent, int igpDestinationIdent)
    {
        ios_fc::Buffer<char> str(message);
        str.grow(1);
        str[str.size() - 1] = 0;
        printf("Message re�u de %d: %s\n", igpOriginIdent, (const char *)message);
    }
};

int main()
{
    int count = 0;
    printf("Hello World!\n");
    try {
        //Socket::setFactory(&unixSocketFactory);
        ios_fc::IGPClient client("localhost", 4567, 25);
        MyListener myListener;
        client.addListener(&myListener);
	while (true) {
            client.idle();
            sleep(1);
            count++;
            if (count == 3) {
                client.sendMessage(25, ios_fc::VoidBuffer("hello", 5));
            }
        }
        //ios_fc::OutputStream *output = testSocket.getOutputStream();
        //output->streamWrite(ios_fc::VoidBuffer("Hello", 5));
    } catch (ios_fc::Exception e) {
        e.printMessage();
    }
    return 0;
}
