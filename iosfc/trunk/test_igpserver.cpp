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
    inline int getIgpIdent() const { return igpID; }
protected:
    void dataReceived(VoidBuffer data);
private:
    void sendIGPIdent();
    IGPServerPortManager *pool;
    int igpID;
    bool valid;
};

class IGPServerPortManager : public ios_fc::StandardServerPortManager {
public:
    IGPServerPortManager();
    void registerConnection(IGPServerConnection *connection);
    void unregisterConnection(IGPServerConnection *connection);
    int getUniqueIGPId();
    bool igpIdValidAndUnique(int igpIdent);
    IGPServerConnection *getConnection(int igpIdent) const;
protected:
    // ServerPortManager implementation
    ServerConnection *createConnection();
private:
    static const int firstAutoIgpIdent;
    int currentAutoIgpIdent;
    AdvancedBuffer<IGPServerConnection *> connections;
};

const int IGPServerPortManager::firstAutoIgpIdent = 32768;

IGPServerPortManager::IGPServerPortManager() : currentAutoIgpIdent(firstAutoIgpIdent)
{
}

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

int IGPServerPortManager::getUniqueIGPId()
{
    return currentAutoIgpIdent++;
}

bool IGPServerPortManager::igpIdValidAndUnique(int igpIdent)
{
    if ((igpIdent <= 0) || (igpIdent >= firstAutoIgpIdent))
        return false;
    for (int i = 0, j = connections.size() ; i < j ; i++) {
        if (igpIdent == connections[i]->getIgpIdent())
            return false;
    }
    return true;
}

IGPServerConnection *IGPServerPortManager::getConnection(int igpIdent) const
{
    for (int i = 0, j = connections.size() ; i < j ; i++) {
        if (igpIdent == connections[i]->getIgpIdent())
            return connections[i];
    }
    return NULL;
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
    //clientSocket->getOutputStream()->streamWrite(VoidBuffer("Hello\n", 6));
}

void IGPServerConnection::dataReceived(VoidBuffer data)
{
    IGPDatagram message(data);
    switch (message.getMsgIdent()) {
    case IGPDatagram::ClientMsgAutoAssignID:
        igpID = pool->getUniqueIGPId();
        printf("Auto-assign ID:%d\n", igpID);
        valid=true;
        sendIGPIdent();
        break;
    case IGPDatagram::ClientMsgAssignID: {
        IGPDatagram::ClientMsgAssignIDDatagram msgReceived(message);
        printf("Assign ID\n");
        if (pool->igpIdValidAndUnique(msgReceived.getIgpIdent())) {
            valid=true;
            igpID = msgReceived.getIgpIdent();
            sendIGPIdent();
        }
        else {
            printf("Adresse igp invalide:%d\n", msgReceived.getIgpIdent());
        }
        break;
    }
    case IGPDatagram::ClientMsgGetID:
        sendIGPIdent();
        break;
    case IGPDatagram::ClientMsgToClient: {
        IGPDatagram::ClientMsgToClientDatagram msgReceived(message);
        IGPDatagram::ServerMsgToClientDatagram msgToSend(igpID, msgReceived.getIgpIdent(), msgReceived.getMessage());
        IGPServerConnection *destConnection = pool->getConnection(msgReceived.getIgpIdent());
        if (destConnection != NULL) {
            destConnection->clientSocket->getOutputStream()->streamWrite(msgToSend.serialize());
        }
        else {
            printf("DEST not found!!!\n");
        }
        //Buffer<char> str(msgToSend.getMessage());
        //str.grow(1);
        //str[str.size() - 1] = 0;
        //printf("Message to %d: %s\n", msgToSend.getIgpIdent(), (const char *)str);
        break;
    }
    default:
        break;
    }
    //Buffer<char> str(data);
    //str.grow(1);
    //str[str.size() - 1] = 0;
    //printf("Donnees recues: %s\n", (const char *)str);
}

void IGPServerConnection::sendIGPIdent()
{
    IGPDatagram::ServerMsgInformIDDatagram reply(igpID);
    clientSocket->getOutputStream()->streamWrite(reply.serialize());
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
