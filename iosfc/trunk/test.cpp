#include "ios_fc.h"
using namespace ios_fc;

int main(int argc, char **argv)
{
  try {
    UDPMessage msg(1);

    msg.addInt("time", 1540);
    msg.addBool("pause", false);
    msg.send();

    return 0;
  }
  catch (Message::DataException e) {
    e.printMessage();
    return 1;
  }
}
