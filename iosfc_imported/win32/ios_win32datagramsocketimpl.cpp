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

#include "ios_win32datagramsocketimpl.h"
#include "ios_win32socketaddressimpl.h"
#include <ws2tcpip.h>

namespace ios_fc {

Win32DatagramSocketImpl::Win32DatagramSocketImpl()
    : broadcastAddress(new Win32SocketAddressImpl(INADDR_BROADCAST)),
      connectedAddress(new Win32SocketAddressImpl(INADDR_ANY))
{
}

void Win32DatagramSocketImpl::create(int localPortNum)
{
    /* grab an Internet domain socket */
    if ((socketFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        throw Exception("IosDatagramSocket: Socket creation failed");
    }

    memset((char *) &boundAddr, 0, sizeof(boundAddr));
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
    if (setsockopt(socketFd, SOL_SOCKET, SO_BROADCAST, (const char *)&optval, sizeof(int)) != -1) {
        hasBroadcast = true;
    }
    isConnected = false;
}

void Win32DatagramSocketImpl::send(Datagram &sendDatagram)
{
    if (!isConnected) {
        Win32SocketAddressImpl *impl = dynamic_cast<Win32SocketAddressImpl *>(sendDatagram.getAddress().getImpl());
        if (impl == NULL)
            throw Exception("Dest address is not compatible with datagramsocket implementation");
        
        struct sockaddr_in outAddr;
        memset((char *) &outAddr, 0, sizeof(outAddr));
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

Datagram Win32DatagramSocketImpl::receive(VoidBuffer buffer)
{
    struct sockaddr_in resultAddress;
    int fromlen = sizeof(resultAddress);
    
    int res = recvfrom(socketFd, (char *)(buffer.ptr()), buffer.size(), 0, (struct sockaddr *) &resultAddress, &fromlen);
    if (res == -1)
        throw Exception("Reception error");
    
    return Datagram(SocketAddress(new Win32SocketAddressImpl(ntohl(resultAddress.sin_addr.s_addr))), ntohs(resultAddress.sin_port), buffer, res);
/*
    int res = recv(socketFd, buffer.ptr(), buffer.size(), 0);
    if (res == -1)
        throw Exception("Reception error");
    return Datagram(connectedAddress, ntohs(connectedPort), buffer, res);
    */
}

int Win32DatagramSocketImpl::available() const
{
    u_long result = 0;
	if (ioctlsocket(socketFd, FIONREAD, &result) == -1) {
		throw Exception("IosSocketStream: ioctl error");
	}
	return (int)result;
return 0;
}

void Win32DatagramSocketImpl::connect(SocketAddress addr, int portNum)
{
    Win32SocketAddressImpl *impl = dynamic_cast<Win32SocketAddressImpl *>(addr.getImpl());
    if (impl == NULL)
        throw Exception("Address is not compatible with datagramsocket implementation");
    
    struct sockaddr_in connectAddr;
    memset((char *) &connectAddr, 0, sizeof(connectAddr));
    connectAddr.sin_family = AF_INET;
    connectAddr.sin_addr.s_addr = htonl(impl->getAddress());
    connectAddr.sin_port = htons(portNum);
    if (::connect(socketFd, (struct sockaddr *) &connectAddr, sizeof(connectAddr)) == -1) {
        throw Exception("Socket connect error");
    }
    isConnected = true;
    connectedAddress = addr;
    connectedPort = portNum;
}

void Win32DatagramSocketImpl::disconnect()
{
    struct sockaddr_in connectAddr;
    memset((char *) &connectAddr, 0, sizeof(connectAddr));
    connectAddr.sin_family = AF_UNSPEC;
    connectAddr.sin_addr.s_addr = 0; // htonl(INADDR_ANY);
    connectAddr.sin_port = htons(0);
    ::connect(socketFd, (struct sockaddr *) &connectAddr, sizeof(connectAddr));
    isConnected = false;
}

void Win32DatagramSocketImpl::joinGroup(SocketAddress groupAddress)
{
    struct ip_mreq mreq;
    Win32SocketAddressImpl *impl = dynamic_cast<Win32SocketAddressImpl *>(groupAddress.getImpl());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    mreq.imr_multiaddr.s_addr = htonl(impl->getAddress());
    setsockopt(socketFd, IPPROTO_IP,IP_ADD_MEMBERSHIP, (const char *) &mreq, sizeof(mreq));
}

void Win32DatagramSocketImpl::setMulticastInterface(SocketAddress interfaceAddress)
{
    Win32SocketAddressImpl *addrImpl = dynamic_cast<ios_fc::Win32SocketAddressImpl *>(interfaceAddress.getImpl());
    in_addr interface_addr;
    interface_addr.s_addr = htonl(addrImpl->getAddress());
    if (setsockopt (socketFd, IPPROTO_IP, IP_MULTICAST_IF, (const char *) &interface_addr, sizeof(in_addr)) != 0)
        throw Exception("setMulticastInterface: setsockopt failed!\n");
}

SocketAddress Win32DatagramSocketImpl::getSocketAddress() const
{
    struct sockaddr_in name;
    int namelen = sizeof(struct sockaddr_in);
    
    int result = getsockname(socketFd, (struct sockaddr *) &name, &namelen);
    if (result != 0) {
        throw Exception("getsockname error");
    }
    return SocketAddress(new Win32SocketAddressImpl(ntohl(name.sin_addr.s_addr)));
}

int Win32DatagramSocketImpl::getSocketPortNum() const
{
    struct sockaddr_in name;
    int namelen = sizeof(struct sockaddr_in);
    
    int result = getsockname(socketFd, (struct sockaddr *) &name, &namelen);
    if (result != 0) {
        throw Exception("getsockname error");
    }
    return ntohs(name.sin_port);
}

Win32DatagramSocketImpl::~Win32DatagramSocketImpl()
{
    closesocket(socketFd);
}

DatagramSocketImpl * Win32DatagramSocketFactory::createDatagramSocket()
{
    return new Win32DatagramSocketImpl();
}

Win32DatagramSocketFactory win32DatagramSocketFactory;

DatagramSocketFactory *DatagramSocket::factory = &win32DatagramSocketFactory;

}
