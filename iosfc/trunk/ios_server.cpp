/* Ultimate Othello 1678
* Copyright (C) 2002  Florent Boudet <flobo@ifrance.com>
* iOS Software <http://ios.free.fr>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*
*/

#include "ios_server.h"
#include "ios_serversocket.h"

namespace ios_fc {

class Server::TaggedItem {
public:
    TaggedItem(ListeningPort *listeningPort) : listeningPort(listeningPort) {}
    enum SocketType {
        ServerSocketType,
        ClientSocketType };
    virtual SocketType getSocketType() = 0;
    ListeningPort *getListeningPort() const { return listeningPort; }
protected:
    ListeningPort *listeningPort;
};

class Server::TaggedSocket : public Socket, public TaggedItem {
public:
    TaggedSocket(SocketImpl *impl, ListeningPort *listeningPort);
    ~TaggedSocket();
    SocketType getSocketType() { return ClientSocketType; }
};

class Server::TaggedClientFactory : public ServerSocket::AcceptedClientFactory {
public:
    TaggedClientFactory(ListeningPort *listeningPort) : listeningPort(listeningPort) {}
    Socket *createSocket(SocketImpl *impl) { return new TaggedSocket(impl, listeningPort); }
protected:
    ListeningPort *listeningPort;
};

class Server::TaggedServerSocket : public ServerSocket, public TaggedItem {
public:
    TaggedServerSocket(int portID, ListeningPort *listeningPort, Selector *selectPool);
    ~TaggedServerSocket();
    SocketType getSocketType() { return ServerSocketType; }
    TaggedSocket *acceptTaggedClient();
private:
    TaggedClientFactory taggedClientFactory;
};

class Server::ListeningPort {
public:
    ListeningPort(int portID, ServerPortManager *manager, Selector *selectPool)
      : portID(portID), manager(manager), selectPool(selectPool), serverSocket(portID, this, selectPool) {}
    Selector *getSelectPool() const { return selectPool; }
    ServerPortManager *getManager() const { return manager; }
    int getPortID() const { return portID; }
private:
    int portID;
    ServerPortManager *manager;
    TaggedServerSocket serverSocket;
    Selector *selectPool;
};


Server::TaggedSocket::TaggedSocket(SocketImpl *impl, ListeningPort *listeningPort)
: Socket(impl), TaggedItem(listeningPort)
{
    listeningPort->getSelectPool()->addSelectable(this);
}

Server::TaggedSocket::~TaggedSocket()
{
    listeningPort->getSelectPool()->removeSelectable(this);
}

Server::TaggedServerSocket::TaggedServerSocket(int portID, ListeningPort *listeningPort, Selector *selectPool)
: ServerSocket(portID), TaggedItem(listeningPort), taggedClientFactory(listeningPort)
{
    selectPool->addSelectable(this);
}

Server::TaggedServerSocket::~TaggedServerSocket()
{
    listeningPort->getSelectPool()->removeSelectable(this);
}

Server::TaggedSocket *Server::TaggedServerSocket::acceptTaggedClient()
{
    TaggedSocket *result = static_cast<TaggedSocket *>(acceptClient(taggedClientFactory));
    return result;
}


Server::Server()
{
}

Server::Server(int portID, ServerPortManager *manager)
{
    addListeningPort(portID, manager);
}

Server::~Server()
{
    for (int i = 0, j = listeningPorts.size() ; i < j ; i++) {
        delete listeningPorts[i];
    }
}

void Server::addListeningPort(int portID, ServerPortManager *manager)
{
    if (getListeningPort(portID) != NULL)
        throw Exception("Port already listening");
    ListeningPort *newPort = new ListeningPort(portID, manager, &selector);
    listeningPorts.add(newPort);
}

void Server::removeListeningPort(int portID)
{
    ListeningPort *portToRemove = getListeningPort(portID);
    if (portToRemove != NULL) {
        listeningPorts.remove(portToRemove);
        delete portToRemove;
    }
}

Server::ListeningPort *Server::getListeningPort(int portID)
{
    for (int i = 0 ; i < listeningPorts.size() ; i++) {
        if (listeningPorts[i]->getPortID() == portID)
            return listeningPorts[i];
    }
    return NULL;
}

void Server::run()
{
    while (true) {
        fflush(stdout);
        selector.select();
        Buffer<Selectable *>awakeSockets = selector.getSelected();
        for (int i = 0, j = awakeSockets.size() ; i < j ; i++) {
            TaggedItem *currentSocket = dynamic_cast<TaggedItem *>(awakeSockets[i]);
            if (currentSocket->getSocketType() == TaggedItem::ServerSocketType) { // New connection
                TaggedServerSocket *serverSocket = dynamic_cast<TaggedServerSocket *>(currentSocket);
                TaggedSocket *client = serverSocket->acceptTaggedClient();
                serverSocket->getListeningPort()->getManager()->connectionFromSocket(client);
            }
            else if (currentSocket->getSocketType() == TaggedItem::ClientSocketType) {
                TaggedSocket *client = dynamic_cast<TaggedSocket *>(currentSocket);
                if (client->getInputStream()->streamAvailable() <= 0) { // deconnection
                    client->getListeningPort()->getManager()->deconnectionFromSocket(client);
                    delete client;
                }
                else try {
                    client->getListeningPort()->getManager()->dataFromSocket(client);
                } catch (Exception e) { // erreur -> deconnection
                    client->getListeningPort()->getManager()->deconnectionFromSocket(client);
                    delete client;
                }
            }
        }
    }
}


};

