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

#ifndef _IOSSERVER
#define _IOSSERVER

#include "ios_memory.h"
#include "ios_selector.h"
#include "ios_socket.h"

namespace ios_fc {

class ServerConnection {
public:
    virtual ~ServerConnection() {}
    void initialize(Socket *client) { clientSocket = client; }
    virtual void connectionMade() = 0;
    virtual void dataReceived() = 0;
    virtual void connectionLost() = 0;
 protected:
    Socket *clientSocket;
};

class StandardServerConnection : public ServerConnection {
public:
    void dataReceived();
    virtual void dataReceived(VoidBuffer *data) = 0;
};

class ServerPortManager {
public:
    virtual void connectionFromSocket(Socket *client) = 0;
    virtual void dataFromSocket(Socket *client) = 0;
    virtual void deconnectionFromSocket(Socket *client) = 0;
};

class StandardServerPortManager : public ServerPortManager {
public:
     void connectionFromSocket(Socket *client);
     void dataFromSocket(Socket *client);
     void deconnectionFromSocket(Socket *client);
private:
     virtual ServerConnection *createConnection() = 0;
 };

class Server {
public:
    Server();
    Server(int portID, ServerPortManager *manager);
    virtual ~Server();
    void addListeningPort(int portID, ServerPortManager *manager);
    void removeListeningPort(int portID);
    void run();
private:
    class ListeningPort;
    class TaggedItem;
    class TaggedServerSocket;
    class TaggedSocket;
    class TaggedClientFactory;
    AdvancedBuffer<ListeningPort *> listeningPorts;
    Selector selector;
    ListeningPort *getListeningPort(int portID);
    friend class StandardServerPortManager; // pas terrible
};

};

#endif // _IOSSERVER

