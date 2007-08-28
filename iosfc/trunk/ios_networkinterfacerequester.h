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

#ifndef _IOSNETWORKINTERFACE
#define _IOSNETWORKINTERFACE

#include <vector>
#include "ios_memory.h"
#include "ios_socketaddress.h"

namespace ios_fc {

class NetworkInterface {
public:
    NetworkInterface(String interfaceName, SocketAddress address) : ifName(interfaceName), ifAddress(address) {}
    virtual String getName() const { return ifName; }
    virtual SocketAddress getAddress() { return ifAddress; }
private:
    String ifName;
    SocketAddress ifAddress;
};

class NetworkInterfaceRequesterImpl {
public:
    virtual std::vector<NetworkInterface> getInterfaces() = 0;
};

class NetworkInterfaceRequesterImplFactory {
public:
    virtual NetworkInterfaceRequesterImpl * createNetworkInterfaceRequesterImpl() = 0;
};

class NetworkInterfaceRequester {
public:
    NetworkInterfaceRequester();
    virtual ~NetworkInterfaceRequester();
    std::vector<NetworkInterface> getInterfaces();
private:
    static NetworkInterfaceRequesterImplFactory *factory;
    NetworkInterfaceRequesterImpl *impl;
};

}

#endif // _IOSNETWORKINTERFACE


