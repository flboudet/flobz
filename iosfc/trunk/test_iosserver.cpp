#include "ios_server.h"
#include <stdio.h>

class MyConnection : public ios_fc::StandardServerConnection {
public:
  void connectionMade();
protected:
  void dataReceived(ios_fc::VoidBuffer *data);
};

class MyPortManager : public ios_fc::StandardServerPortManager {
protected:
  ios_fc::ServerConnection *createConnection();
};


ios_fc::ServerConnection *MyPortManager::createConnection()
{
  return new MyConnection();
}

void MyConnection::connectionMade()
{
  //  clientSocket->getOutputStream()->streamWrite(ios_fc::Buffer<char>("Hello\n"));
}

void MyConnection::dataReceived(ios_fc::VoidBuffer *data)
{
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
