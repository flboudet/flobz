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

#ifndef _IGPMESSAGE_H
#define _IGPMESSAGE_H

#include "ios_standardmessage.h"
#include "ios_dirigeable.h"

namespace ios_fc {
    class IgpMessage : public StandardMessage, public Dirigeable {
    public:
        IgpMessage(int serialID, int igpPeerIdent);
        IgpMessage(const Buffer<char> serialized, int igpPeerIdent) throw(InvalidMessageException);
        // Dirigeable
        PeerAddress getPeerAddress();
        PeerAddress getBroadcastAddress();
        void setPeerAddress(PeerAddress);
        void addPeerAddress(const String key, const PeerAddress &value);
        PeerAddress getPeerAddress(const String key);
        class IgpPeerAddressImpl; // A revoir
    protected:
        int igpPeerIdent;
    };

    class IgpMessage::IgpPeerAddressImpl : public PeerAddressImpl {
    public:
        IgpPeerAddressImpl(int igpIdent) : igpIdent(igpIdent) {}
        virtual bool operator == (const PeerAddressImpl &a) const {
            const IgpPeerAddressImpl &comp = dynamic_cast<const IgpPeerAddressImpl &>(a);
            return (comp.igpIdent == igpIdent);
        }
        inline int getIgpIdent() const { return igpIdent; }
    private:
        int igpIdent;
    };
}

#endif // _IGPMESSAGE_H

