/**
 * iosfc::UnixDatagramSocket: a BSD Sockets implementation of datagramsocket
 * 
 * This file is part of the iOS Foundation Classes project.
 *
 * authors:
 *  Jean-Christophe Hoelt <jeko@ios-software.com>
 *  Guillaume Borios      <gyom@ios-software.com>
 *  Florent Boudet       <flobo@ios-software.com>
 *  Michel Metzger -- participate during the developement of Shubunkin!
 *
 * http://www.ios-software.com/
 *
 * Released under the terms of the GNU General Public Licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */
 
#ifndef _IOSUNIXDATAGRAMSOCKETIMPL
#define _IOSUNIXDATAGRAMSOCKETIMPL
 
#include "ios_datagramsocket.h"
#include "ios_unixselectorimpl.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

namespace ios_fc {
    
    class UnixDatagramSocketImpl : public DatagramSocketImpl, public UnixSelectableImpl {
    public:
        UnixDatagramSocketImpl();
        virtual ~UnixDatagramSocketImpl();
        void create(int localPortNum);
        void send(Datagram &sendDatagram);
        Datagram receive(VoidBuffer buffer);
        int available() const;
        void connect(SocketAddress addr, int portNum);
        void disconnect();
        SocketAddress getBroadcastAddress() const { return broadcastAddress; }
        SocketAddress getSocketAddress() const;
        int getSocketPortNum() const;
        SelectableImpl *getSelectableImpl() { return this; }
        int getFd() { return socketFd; }
	bool getConnected() { return isConnected; }
	SocketAddress getConnectedAddress() { return connectedAddress; }
	int getConnectedPortNum() { return connectedPort; }
    private:
        int socketFd;
        struct sockaddr_in boundAddr;
        bool hasBroadcast;
        SocketAddress broadcastAddress;
        bool isConnected;
        SocketAddress connectedAddress;
        int connectedPort;
    };

    class UnixDatagramSocketFactory : public DatagramSocketFactory {
    public:
        DatagramSocketImpl * createDatagramSocket();
    };
}

#endif // _IOSUNIXDATAGRAMSOCKETIMPL

