#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "ios_udpmessagebox.h"
using namespace ios_fc;
#include "SDL.h"

class PuyoNetwork : public MessageListener
{
  public:
    PuyoNetwork(MessageBox *mbox) : mbox(mbox) {
      mbox->addListener(*this);
    }
    
    void onMessage(Message &message) {
      if (message.getString("type") == String("play")) {
        printf("%s wants to play !\n", (const char*)message.getString("name"));
      }
      if (message.getString("type") == String("pok")) {
        printf("%s is ok to play !\n", (const char*)message.getString("name"));
      }
      if (message.getString("type") == String("pnok")) {
        printf("%s doesn't want to play !\n", (const char*)message.getString("name"));
      }
    }

    void idle() {
      mbox->idle();
    }

    void wannaPlay() {
      Message *msg = mbox->createMessage();
      msg->addBoolProperty("reliable", true);
      msg->addString("type", "play");
      msg->addString("name", "MonNom");
      msg->addInt("game_level", 1);
      msg->send();
    }

    void okToPlay() {
      Message *msg = mbox->createMessage();
      msg->addBoolProperty("reliable", true);
      msg->addString("type", "pok");
      msg->addString("name", "MonNom");
      msg->send();
    }

    void notOkToPlay() {
      Message *msg = mbox->createMessage();
      msg->addBoolProperty("reliable", true);
      msg->addString("type", "pnok");
      msg->addString("name", "MonNom");
      msg->send();
    }
    
  private:
    MessageBox *mbox;
};

int main(int argc, char *argv[])
{
  UDPMessageBox toto(argv[argc-1], 6581, 6581);
  PuyoNetwork network(&toto);

  while (true) {
    char line[256];
    gets(line);
    if (!strncmp(line, "play", 4)) {
      network.wannaPlay();
    }
    if (!strncmp(line, "ok", 2)) {
      network.okToPlay();
    }
    if (!strncmp(line, "nok", 3)) {
      network.notOkToPlay();
    }
    
    network.idle();
  }

  return 0;
}
