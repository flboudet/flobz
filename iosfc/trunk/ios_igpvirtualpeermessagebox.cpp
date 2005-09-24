/**
 * iosfc::IgpVirtualPeerMessageBox: a messagebox designed as a virtual peer
 * for the IgpMessageListener. Handy to implement a custom service on an IGP
 * server.
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

#include "ios_igpvirtualpeermessagebox.h"
#include "ios_igpmessage.h"

namespace ios_fc {

class IgpVirtualPeerMessage : public IgpMessage {
public:
    IgpVirtualPeerMessage(int serialID, IgpVirtualPeerMessageBox &owner, int igpPeerIdent);
    IgpVirtualPeerMessage(const Buffer<char> serialized, IgpVirtualPeerMessageBox &owner, int igpPeerIdent) throw(InvalidMessageException);
    void sendBuffer(Buffer<char> out) const;
private:
    IgpVirtualPeerMessageBox &owner;
};


IgpVirtualPeerMessage::IgpVirtualPeerMessage(int serialID, IgpVirtualPeerMessageBox &owner, int igpPeerIdent) : IgpMessage(serialID, igpPeerIdent), owner(owner)
{
}

IgpVirtualPeerMessage::IgpVirtualPeerMessage(const Buffer<char> serialized, IgpVirtualPeerMessageBox &owner, int igpPeerIdent) throw(InvalidMessageException)
: IgpMessage(serialized, igpPeerIdent), owner(owner)
{
}

void IgpVirtualPeerMessage::sendBuffer(Buffer<char> out) const
{
    owner.sendBuffer(out, isReliable(), igpPeerIdent);
}

// IgpVirtualPeerMessageBox

IgpVirtualPeerMessageBox::IgpVirtualPeerMessageBox(IgpMessageListener &igpListener, int igpIdent)
    : IgpVirtualPeer(&igpListener, igpIdent), sendSerialID(0)
{
}

IgpVirtualPeerMessageBox::IgpVirtualPeerMessageBox(IgpMessageListener &igpListener)
    : IgpVirtualPeer(&igpListener), sendSerialID(0)
{
}

// Implement MessageBox
Message * IgpVirtualPeerMessageBox::createMessage()
{
    return new IgpVirtualPeerMessage(++sendSerialID, *this, destIdent);
}

// Implement IgpPeer
void IgpVirtualPeerMessageBox::messageReceived(VoidBuffer message, int origIgpIdent, bool reliable)
{
    try {
        IgpVirtualPeerMessage incomingMessage(message, *this, origIgpIdent);
        for (int i = 0, j = listeners.size() ; i < j ; i++) {
            MessageListener *currentListener = listeners[i];
            currentListener->onMessage(incomingMessage);
        }
    }
    catch (Exception e) {
        e.printMessage();
    }
}

// Own member functions
void IgpVirtualPeerMessageBox::sendBuffer(VoidBuffer out, bool reliable, int igpDestIdent)
{
    sendMessageToAddress(out, igpDestIdent, reliable);
}

void IgpVirtualPeerMessageBox::bind(PeerAddress addr)
{
    IgpMessage::IgpPeerAddressImpl *peerAddressImpl = dynamic_cast<IgpMessage::IgpPeerAddressImpl *>(addr.getImpl());
    if (peerAddressImpl != NULL) {
        destIdent = peerAddressImpl->getIgpIdent();
    }
    else throw Exception("Incompatible peer address type!");
}

}
