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
    : broadcastAddress(new UnixSocketAddressImpl(INADDR_BROADCAST))
{
}

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
}

void UnixDatagramSocketImpl::send(Datagram &sendDatagram)
{
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
