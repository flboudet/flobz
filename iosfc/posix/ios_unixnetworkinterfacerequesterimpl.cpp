/**
 * iosfc::NetworkInterfaceRequester: Class to request a list of 
 * or an IgpVirtualPeerMessageBox
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
 
#include "ios_networkinterfacerequester.h"
#include <stdio.h>
#include "ios_datagramsocket.h"
#include "ios_selector.h"
#include <unistd.h>

#include "ios_unixdatagramsocketimpl.h"
#include "ios_unixsocketaddressimpl.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

using namespace std;

namespace ios_fc {

class UnixNetworkInterfaceRequesterImpl : public NetworkInterfaceRequesterImpl {
    virtual vector<NetworkInterface> getInterfaces();
};

vector<NetworkInterface> UnixNetworkInterfaceRequesterImpl::getInterfaces()
{
    vector<NetworkInterface> result;
    struct ifconf ifc;
    const int MAX_IFS = 32;
    char buf[sizeof(struct ifreq)*MAX_IFS];
    ifc.ifc_len = sizeof( buf );
    ifc.ifc_buf = (caddr_t)buf;
    
    ios_fc::DatagramSocket toto;
    ios_fc::UnixDatagramSocketImpl &impl = dynamic_cast<ios_fc::UnixDatagramSocketImpl &>(toto.getImpl());
    ioctl(impl.getFd(), SIOCGIFCONF, (caddr_t)&ifc);
    
    ifreq *curIf = (ifreq *)(ifc.ifc_buf);
    int remaining = ifc.ifc_len;

    while (remaining) {
        struct sockaddr_in *addr = (struct sockaddr_in *)&(curIf->ifr_addr);
        if (addr->sin_family == AF_INET) {
            result.push_back(NetworkInterface(String(curIf->ifr_name), SocketAddress(inet_ntoa(addr->sin_addr))));
        }
#ifdef LINUX
        int current = sizeof(ifreq);
#else
        int current = curIf->ifr_addr.sa_len + IFNAMSIZ;
#endif
        curIf = (struct ifreq *)( ((char *)curIf)+current );
        remaining -= current;
    }
    return result;
}

class UnixNetworkInterfaceRequesterImplFactory : public NetworkInterfaceRequesterImplFactory {
public:
    virtual NetworkInterfaceRequesterImpl * createNetworkInterfaceRequesterImpl();
};

NetworkInterfaceRequesterImpl * UnixNetworkInterfaceRequesterImplFactory::createNetworkInterfaceRequesterImpl()
{
    return new UnixNetworkInterfaceRequesterImpl();
}

UnixNetworkInterfaceRequesterImplFactory unixNetworkInterfaceRequesterImplFactory;

NetworkInterfaceRequesterImplFactory *NetworkInterfaceRequester::factory = &unixNetworkInterfaceRequesterImplFactory;

}

