#include "ios_fc.h"
using namespace ios_fc;

int main(int argc, char **argv)
{
  try {
    UDPMessage msg(1);
    static int tab[6] = { 1, 2, 3, 4, 5, 6};

    msg.addInt("time", 1540);
    msg.addBool("pause", false);
    msg.addString("name", "Jeko");
    msg.addIntArray("tab", Buffer<int>(tab,6));
    
    Buffer<char> serialized = msg.serialize();
    printf("SERIALIZED: size(%d)\n%s\n",
           serialized.size(), (const char*)serialized);
    
    msg.send();

    return 0;
  }
  catch (Message::DataException e) {
    e.printMessage();
    return 1;
  }
}
