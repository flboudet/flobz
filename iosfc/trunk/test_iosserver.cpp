#include "ios_server.h"
#include <stdio.h>

class MyPortManager : public ios_fc::ServerPortManager {
public:
  void connectionFromSocket(ios_fc::Socket *client);
  void dataFromSocket(ios_fc::Socket *client);
  void deconnectionFromSocket(ios_fc::Socket *client);
};

void MyPortManager::connectionFromSocket(ios_fc::Socket *client)
{
  printf("New conection\n");
}

void MyPortManager::dataFromSocket(ios_fc::Socket *client)
{
  printf("New data\n");
}

void MyPortManager::deconnectionFromSocket(ios_fc::Socket *client)
{
  printf("Deconnection\n");
}


int main()
{
  ios_fc::Server server;
  MyPortManager manager;

  try {
    server.addListeningPort(4567, &manager);
    server.run();
  }
  catch (ios_fc::Exception e) {
    e.printMessage();
  }
  return 0;
}
