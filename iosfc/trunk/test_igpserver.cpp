#include "ios_server.h"
#include "ios_igpdatagram.h"
#include <stdio.h>

namespace ios_fc {

class IGPServerPortManager;

class IGPServerConnection : public StandardServerConnection {
public:
    IGPServerConnection(IGPServerPortManager *pool);
    ~IGPServerConnection();
    void connectionMade();
protected:
    void dataReceived(VoidBuffer data);
private:
    IGPServerPortManager *pool;
    int igpID;
    bool valid;
};

class IGPServerPortManager : public ios_fc::StandardServerPortManager {
public:
    void registerConnection(IGPServerConnection *connection);
    void unregisterConnection(IGPServerConnection *connection);
    int getUniqueIGPId();
protected:
    // ServerPortManager implementation
    ServerConnection *createConnection();
private:
    AdvancedBuffer<IGPServerConnection *> connections;
};


ServerConnection *IGPServerPortManager::createConnection()
{
    return new IGPServerConnection(this);
}

void IGPServerPortManager::registerConnection(IGPServerConnection *connection)
{
    connections.add(connection);
}

void IGPServerPortManager::unregisterConnection(IGPServerConnection *connection)
{
    connections.remove(connection);
}

IGPServerConnection::IGPServerConnection(IGPServerPortManager *pool) : pool(pool)
{
    igpID = 0;
    valid = false;
    pool->registerConnection(this);
}

IGPServerConnection::~IGPServerConnection()
{
    pool->unregisterConnection(this);
}

void IGPServerConnection::connectionMade()
{
    clientSocket->getOutputStream()->streamWrite(VoidBuffer("Hello\n", 6));
}

void IGPServerConnection::dataReceived(VoidBuffer data)
{
    IGPDatagram message(data);
    switch (message.getMsgIdent()) {
    case IGPDatagram::ClientMsgAutoAssignID:
        printf("Auto-assign ID\n");
        break;
    default:
        break;
    }
    //Buffer<char> str(data);
    //str.grow(1);
    //str[str.size() - 1] = 0;
    //printf("Donnees recues: %s\n", (const char *)str);
}

};

int main()
{
  ios_fc::Server server;
  ios_fc::IGPServerPortManager manager;

  try {
    server.addListeningPort(4567, &manager);
    server.run();
  }
  catch (ios_fc::Exception e) {
    e.printMessage();
  }
  return 0;
}