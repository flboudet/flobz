#include "ios_server.h"
#include <stdio.h>

class MyConnection : public ios_fc::StandardServerConnection {
public:
  void connectionMade();
protected:
  void dataReceived(ios_fc::VoidBuffer data);
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
  clientSocket->getOutputStream()->streamWrite(ios_fc::VoidBuffer("Hello\n", 6));
}

void MyConnection::dataReceived(ios_fc::VoidBuffer data)
{
    ios_fc::Buffer<char> str(data);
    str.grow(1);
    str[str.size() - 1] = 0;
    printf("Donnees recues: %s\n", (const char *)str);
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
