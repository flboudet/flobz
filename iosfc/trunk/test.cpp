#include "ios_fc.h"
#include "ios_igpdatagram.h"
using namespace ios_fc;

Buffer<char> message(100);

    void writeBigEndianIntToMessage(int integer, int offset) {
        message[offset]     = (integer & 0xFF000000) >> 24;
        message[offset + 1] = (integer & 0x00FF0000) >> 16;
        message[offset + 2] = (integer & 0x0000FF00) >> 8;
        message[offset + 3] = (integer & 0x000000FF);
    }

    int readBigEndianIntFromMessage(int offset) {
        return ((unsigned char)message[offset] << 24)
          | ((unsigned char)message[offset + 1] << 16)
          | ((unsigned char)message[offset + 2] << 8)
          | (unsigned char)message[offset + 3];
    }

void test_memory()
{
    writeBigEndianIntToMessage(0x12345678, 12);
    printf("%08x = 12345678\n", readBigEndianIntFromMessage(12));

    String s = "test";
    printf ("%s = test et quoi?\n", (const char *)(s + " et quoi?"));

    s += "yo";
    printf ("%s = testyo et quoi?\n", (const char *)(s + " et quoi?"));

    printf("%d = 100\n", message.size());
    message += 10;
}

int main(int argc, char **argv)
{
  try {
    //  test_memory();
    StandardMessage msg(1);
    static int  tabi[6] = { 1, 2, 3, 4, 5, 6};
    static char tabc[6] = { 1, 2, 3, 4, 5, -12};

    msg.addInt("time", 1540);
    msg.addBool("pause", false);
    msg.addString("name", "Jeko");
    msg.addIntArray("tabi", Buffer<int>(tabi,6));
    msg.addCharArray("tabc", Buffer<char>(tabc,6));
    
    Buffer<char> serialized = msg.serialize();
    printf("SERIALIZED: size(%d)\n%s\n",
           serialized.size(), (const char*)serialized);
    
//    msg.send();

    StandardMessage test(serialized);
    printf("\nBIS\n%s\n", (const char *)test.serialize());

    return 0;
  }
  catch (Message::DataException e) {
    e.printMessage();
    return 1;
  }
}
