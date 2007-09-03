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

#include "ios_win32datagramsocketimpl.h"
#include "ios_win32socketaddressimpl.h"
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

namespace ios_fc {

class Win32NetworkInterfaceRequesterImpl : public NetworkInterfaceRequesterImpl {
    virtual vector<NetworkInterface> getInterfaces();
};

#define MAXINTF 100
vector<NetworkInterface> Win32NetworkInterfaceRequesterImpl::getInterfaces()
{
    vector<NetworkInterface> result;
    INTERFACE_INFO ifinfo[MAXINTF];
    DWORD bytesReturned = 0;
    ios_fc::DatagramSocket toto;
    ios_fc::Win32DatagramSocketImpl &impl = dynamic_cast<ios_fc::Win32DatagramSocketImpl &>(toto.getImpl());
    WSAIoctl(impl.getSocket(), SIO_GET_INTERFACE_LIST, NULL,
	     0, &ifinfo, sizeof(INTERFACE_INFO) * MAXINTF, &bytesReturned, NULL, NULL);
    
    for (int i = 0 ; i < (bytesReturned / sizeof(INTERFACE_INFO)) ; i++) {
      INTERFACE_INFO *curIf = &(ifinfo[i]);
      struct sockaddr_in *addr = (struct sockaddr_in *)&(curIf->iiAddress);
      if (addr->sin_family == AF_INET) {
	result.push_back(NetworkInterface(String("Not implemented"), SocketAddress(inet_ntoa(addr->sin_addr))));
      }
    }
    return result;
}

class Win32NetworkInterfaceRequesterImplFactory : public NetworkInterfaceRequesterImplFactory {
public:
    virtual NetworkInterfaceRequesterImpl * createNetworkInterfaceRequesterImpl();
};

NetworkInterfaceRequesterImpl * Win32NetworkInterfaceRequesterImplFactory::createNetworkInterfaceRequesterImpl()
{
    return new Win32NetworkInterfaceRequesterImpl();
}

Win32NetworkInterfaceRequesterImplFactory w32NetworkInterfaceRequesterImplFactory;

NetworkInterfaceRequesterImplFactory *NetworkInterfaceRequester::factory = &w32NetworkInterfaceRequesterImplFactory;

}

