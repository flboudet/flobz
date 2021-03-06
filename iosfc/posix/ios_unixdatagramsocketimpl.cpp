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

#include "ios_unixdatagramsocketimpl.h"
#include "ios_unixsocketaddressimpl.h"
#include <unistd.h>
#include <sys/ioctl.h>

#ifdef NO_SOCKLEN_T
#define socklen_t int
#endif

namespace ios_fc {

UnixDatagramSocketImpl::UnixDatagramSocketImpl()
    : broadcastAddress(new UnixSocketAddressImpl(INADDR_BROADCAST)),
      connectedAddress(new UnixSocketAddressImpl(INADDR_ANY))
{}

void UnixDatagramSocketImpl::create(int localPortNum)
{
    /* grab an Internet domain socket */
    if ((socketFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        throw Exception("IosDatagramSocket: Socket creation failed");
    }
    
    bzero((char *) &boundAddr, sizeof(boundAddr));
    boundAddr.sin_family = AF_INET;
    boundAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    boundAddr.sin_port = htons((localPortNum == DatagramSocket::NODEFINEDPORT) ? 0 : localPortNum);
    
    // bind the socket as appropriate
    if (bind(socketFd, (struct sockaddr *) &boundAddr, sizeof(boundAddr)) == -1) {
        throw Exception("Socket binding error");
    }
    
    // enable broadcast on the socket
    int optval = 1;
    hasBroadcast = false;
    if (setsockopt(socketFd, SOL_SOCKET, SO_BROADCAST, (const void *)&optval, sizeof(int)) != -1) {
        hasBroadcast = true;
    }
    isConnected = false;
}

void UnixDatagramSocketImpl::send(Datagram &sendDatagram)
{
    if (!isConnected) {
        UnixSocketAddressImpl *impl = dynamic_cast<UnixSocketAddressImpl *>(sendDatagram.getAddress().getImpl());
        if (impl == NULL)
            throw Exception("Dest address is not compatible with datagramsocket implementation");
        
        struct sockaddr_in outAddr;
        bzero((char *) &outAddr, sizeof(outAddr));
        outAddr.sin_family = AF_INET;
        outAddr.sin_addr.s_addr = htonl(impl->getAddress());
        outAddr.sin_port = htons(sendDatagram.getPortNum());
        
        sendto(socketFd, sendDatagram.getMessage(), sendDatagram.getSize(), 0,
               (struct sockaddr *) &outAddr, sizeof(outAddr));
    }
    else {
        ::send(socketFd, sendDatagram.getMessage(), sendDatagram.getSize(), 0);
    }
}

Datagram UnixDatagramSocketImpl::receive(VoidBuffer buffer)
{
    struct sockaddr_in resultAddress;
    socklen_t fromlen = sizeof(resultAddress);
    
    int res = recvfrom(socketFd, buffer.ptr(), buffer.size(), 0, (struct sockaddr *) &resultAddress, &fromlen);
    if (res == -1)
        throw Exception("Reception error");
    
    return Datagram(SocketAddress(new UnixSocketAddressImpl(ntohl(resultAddress.sin_addr.s_addr))), ntohs(resultAddress.sin_port), buffer, res);
}

int UnixDatagramSocketImpl::available() const
{
    int result = 0;
	if (ioctl(socketFd, FIONREAD, &result) == -1) {
		throw Exception("IosSocketStream: ioctl error");
	}
	return result;
}

void UnixDatagramSocketImpl::connect(SocketAddress addr, int portNum)
{
    UnixSocketAddressImpl *impl = dynamic_cast<UnixSocketAddressImpl *>(addr.getImpl());
    if (impl == NULL)
        throw Exception("Address is not compatible with datagramsocket implementation");
    
    struct sockaddr_in connectAddr;
    bzero((char *) &connectAddr, sizeof(connectAddr));
    connectAddr.sin_family = AF_INET;
    connectAddr.sin_addr.s_addr = htonl(impl->getAddress());
    connectAddr.sin_port = htons(portNum);
    if (::connect(socketFd, (struct sockaddr *) &connectAddr, (socklen_t)sizeof(connectAddr)) == -1) {
        throw Exception("Socket connect error");
    }
    isConnected = true;
    connectedAddress = addr;
    connectedPort = portNum;
}

void UnixDatagramSocketImpl::disconnect()
{
#ifdef LINUX
    // We have to keep the information of the currently bound address (because of Linux behaviour)
    struct sockaddr_in boundAddr;
    socklen_t namelen = sizeof(struct sockaddr_in);
    int result = getsockname(socketFd, (struct sockaddr *) &boundAddr, &namelen);
    if (result != 0) {
        throw Exception("getsockname error");
    }
#endif

    // Disconnect the socket
    struct sockaddr_in connectAddr;
    bzero((char *) &connectAddr, sizeof(connectAddr));
    connectAddr.sin_family = AF_UNSPEC;
    connectAddr.sin_addr.s_addr = 0;//htonl(INADDR_ANY);
    connectAddr.sin_port = htons(0);
    ::connect(socketFd, (struct sockaddr *) &connectAddr, sizeof(connectAddr));

#ifdef LINUX
    // We have to rebind the socket to its previous port, because Linux unbinds the socket when disconnecting
    if (bind(socketFd, (struct sockaddr *) &boundAddr, sizeof(boundAddr)) == -1) {
        throw Exception("Socket binding error");
    }
#endif
    isConnected = false;
}

void UnixDatagramSocketImpl::joinGroup(SocketAddress groupAddress)
{
    struct ip_mreq mreq;
    UnixSocketAddressImpl *impl = dynamic_cast<UnixSocketAddressImpl *>(groupAddress.getImpl());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    mreq.imr_multiaddr.s_addr = htonl(impl->getAddress());
    setsockopt(socketFd, IPPROTO_IP,IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
}

void UnixDatagramSocketImpl::setMulticastInterface(SocketAddress interfaceAddress)
{
    UnixSocketAddressImpl *addrImpl = dynamic_cast<ios_fc::UnixSocketAddressImpl *>(interfaceAddress.getImpl());
    in_addr_t interface_addr = htonl(addrImpl->getAddress());
    if (setsockopt (socketFd, IPPROTO_IP, IP_MULTICAST_IF, &interface_addr, sizeof(in_addr)) != 0)
        throw Exception("setMulticastInterface: setsockopt failed!\n");
}

SocketAddress UnixDatagramSocketImpl::getSocketAddress() const
{
    struct sockaddr_in name;
    socklen_t namelen = sizeof(struct sockaddr_in);
    
    int result = getsockname(socketFd, (struct sockaddr *) &name, &namelen);
    if (result != 0) {
        throw Exception("getsockname error");
    }
    return SocketAddress(new UnixSocketAddressImpl(ntohl(name.sin_addr.s_addr)));
}

int UnixDatagramSocketImpl::getSocketPortNum() const
{
    struct sockaddr_in name;
    socklen_t namelen = sizeof(struct sockaddr_in);
    
    int result = getsockname(socketFd, (struct sockaddr *) &name, &namelen);
    if (result != 0) {
        throw Exception("getsockname error");
    }
    return ntohs(name.sin_port);
}

UnixDatagramSocketImpl::~UnixDatagramSocketImpl()
{
    close(socketFd);
}

DatagramSocketImpl * UnixDatagramSocketFactory::createDatagramSocket()
{
    return new UnixDatagramSocketImpl();
}

UnixDatagramSocketFactory unixDatagramSocketFactory;

DatagramSocketFactory *DatagramSocket::factory = &unixDatagramSocketFactory;

}
