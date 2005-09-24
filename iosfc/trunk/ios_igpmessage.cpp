/**
 * iosfc::IgpMessage: Incarnation of the Message for an IgpMessageBox
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

#include "ios_igpmessage.h"

namespace ios_fc {
    
    IgpMessage::IgpMessage(int serialID, int igpPeerIdent) : StandardMessage(serialID), igpPeerIdent(igpPeerIdent)
    {
    }

    IgpMessage::IgpMessage(const Buffer<char> serialized, int igpPeerIdent) throw(InvalidMessageException)
    : StandardMessage(serialized), igpPeerIdent(igpPeerIdent)
    {
    }
    
    // Dirigeable
    PeerAddress IgpMessage::getPeerAddress()
    {
        return PeerAddress(new IgpPeerAddressImpl(igpPeerIdent));
    }
    
    PeerAddress IgpMessage::getBroadcastAddress()
    {
        return PeerAddress(new IgpPeerAddressImpl(0));
    }
    
    void IgpMessage::setPeerAddress(PeerAddress newPeerAddress)
    {
        IgpPeerAddressImpl *newPeerAddressImpl = dynamic_cast<IgpPeerAddressImpl *>(newPeerAddress.getImpl());
        if (newPeerAddressImpl != NULL) {
            igpPeerIdent = newPeerAddressImpl->getIgpIdent();
        }
        else throw Exception("Incompatible peer address type!");
    }
    
    void IgpMessage::addPeerAddress(const String key, const PeerAddress &value)
    {
        IgpPeerAddressImpl *peerAddressImpl = dynamic_cast<IgpPeerAddressImpl *>(value.getImpl());
        if (peerAddressImpl != NULL) {
            addInt(key, peerAddressImpl->getIgpIdent());
        }
        else throw Exception("Incompatible peer address type!");
    }
    
    PeerAddress IgpMessage::getPeerAddress(const String key)
    {
        return PeerAddress(new IgpPeerAddressImpl(getInt(key)));
    }

}

