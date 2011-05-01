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

    template <typename T>
    class IgpMessage : public T, public Dirigeable {
    public:
        IgpMessage(int serialID, int igpPeerIdent);
        IgpMessage(const Buffer<char> serialized, int igpPeerIdent) throw (Message::InvalidMessageException);
        // Dirigeable
        PeerAddress getPeerAddress();
        PeerAddress getBroadcastAddress();
        void setPeerAddress(PeerAddress);
        void addPeerAddress(const String key, const PeerAddress &value);
        PeerAddress getPeerAddress(const String key);
    protected:
        int igpPeerIdent;
        static const char * SERIAL_ID;//   = "SID";
    };

    class IgpPeerAddressImpl : public PeerAddressImpl {
    public:
        IgpPeerAddressImpl(int igpIdent) : igpIdent(igpIdent) {}
        virtual bool operator == (const PeerAddressImpl &a) const {
            const IgpPeerAddressImpl &comp = dynamic_cast<const IgpPeerAddressImpl &>(a);
            return (comp.igpIdent == igpIdent);
        }
        virtual bool operator < (const PeerAddressImpl &a) const {
            const IgpPeerAddressImpl &comp = dynamic_cast<const IgpPeerAddressImpl &>(a);
            return (igpIdent < comp.igpIdent);
        }
        inline int getIgpIdent() const { return igpIdent; }
    private:
        int igpIdent;
    };
    
    template <typename T>
    const char * IgpMessage<T>::SERIAL_ID = "SID";

    template <typename T>
    IgpMessage<T>::IgpMessage(int serialID, int igpPeerIdent) : igpPeerIdent(igpPeerIdent)
    {
        T::addIntProperty(SERIAL_ID, serialID);
    }
    
    template <typename T>
    IgpMessage<T>::IgpMessage(const Buffer<char> serialized, int igpPeerIdent) throw(Message::InvalidMessageException)
    : T(serialized), igpPeerIdent(igpPeerIdent)
    {
    }
    
    // Dirigeable
    template <typename T>
    PeerAddress IgpMessage<T>::getPeerAddress()
    {
        return PeerAddress(new IgpPeerAddressImpl(igpPeerIdent));
    }
    
    template <typename T>
    PeerAddress IgpMessage<T>::getBroadcastAddress()
    {
        return PeerAddress(new IgpPeerAddressImpl(0));
    }
    
    template <typename T>
    void IgpMessage<T>::setPeerAddress(PeerAddress newPeerAddress)
    {
        IgpPeerAddressImpl *newPeerAddressImpl = dynamic_cast<IgpPeerAddressImpl *>(newPeerAddress.getImpl());
        if (newPeerAddressImpl != NULL) {
            igpPeerIdent = newPeerAddressImpl->getIgpIdent();
        }
        else throw Exception("Incompatible peer address type!");
    }
    
    template <typename T>
    void IgpMessage<T>::addPeerAddress(const String key, const PeerAddress &value)
    {
        IgpPeerAddressImpl *peerAddressImpl = dynamic_cast<IgpPeerAddressImpl *>(value.getImpl());
        if (peerAddressImpl != NULL) {
            T::addInt(key, peerAddressImpl->getIgpIdent());
        }
        else throw Exception("Incompatible peer address type!");
    }
    
    template <typename T>
    PeerAddress IgpMessage<T>::getPeerAddress(const String key)
    {
        return PeerAddress(new IgpPeerAddressImpl(T::getInt(key)));
    }
}

#endif // _IGPMESSAGE_H

