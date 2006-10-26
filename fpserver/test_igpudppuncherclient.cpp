#include "ios_udpmessagebox.h"
#include "ios_igpmessagebox.h"
#include "ios_udpmessage.h"
#include "ios_time.h"
#include "PuyoIgpDefs.h"
#include "PuyoNatTraversal.h"
#include <unistd.h>

using namespace ios_fc;

int main(int argc, char *argv[])
{
    UDPMessageBox udpmbox(argv[1], 0, atoi(argv[2]));
    PuyoNatTraversal toto(udpmbox);
    toto.punch(argv[3]);
    
    while ((!toto.hasFailed()) && (!toto.hasSucceeded())) {
        usleep(20000);
        toto.idle();
    }
    if (toto.hasFailed()) {
        printf("FAILED to traverse NAT\n");
    }
    if (toto.hasSucceeded()) {
        printf("SUCCEEDED to traverse NAT\n");
    }
    if (toto.hasSucceeded()) {
        for (int i = 0 ; i < 100 ; i++)
        {
            usleep(20000);
            udpmbox.idle();
        }
    }
    return 0;
}

