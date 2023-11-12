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

#ifndef _IGPVIRTUALPEERMESSAGEBOX_H
#define _IGPVIRTUALPEERMESSAGEBOX_H

#include "ios_messagebox.h"
#include "ios_igpmessagelistener.h"
#include "ios_igpmessage.h"

namespace ios_fc {

template <typename T>
class IgpVirtualPeerMessageBox : public MessageBox, private IgpVirtualPeer {
public:
    IgpVirtualPeerMessageBox(IgpMessageListener &igpListener, int igpIdent);
    IgpVirtualPeerMessageBox(IgpMessageListener &igpListener);
    // Implement MessageBox
    // The idle() function is useless, since the messagebox is virtual and
    // uses the IOs from the IgpMessageListener
    void idle() {}
    Message * createMessage();
    // Implement IgpPeer
    void messageReceived(VoidBuffer message, int origIgpIdent, bool reliable);
    // Own member functions
    void sendBuffer(VoidBuffer out, bool reliable, int igpDestIdent);
    void bind(int igpIdent) { destIdent = igpIdent; }
    void bind(PeerAddress addr); // a revoir
    int getBound() const { return destIdent; }
private:
    int sendSerialID;
    int destIdent;
    bool m_isCorrupted;
};

template <typename T>
class IgpVirtualPeerMessage : public IgpMessage<T> {
public:
    IgpVirtualPeerMessage(int serialID, IgpVirtualPeerMessageBox<T> &owner, int igpPeerIdent);
    IgpVirtualPeerMessage(const Buffer<char> serialized, IgpVirtualPeerMessageBox<T> &owner, int igpPeerIdent);
    virtual void send();
    bool isReliable() const {
        if (T::hasBoolProperty("RELIABLE"))
            return T::getBoolProperty("RELIABLE");
        return false;
    }
private:
    IgpVirtualPeerMessageBox<T> &owner;
};

template <typename T>
IgpVirtualPeerMessage<T>::IgpVirtualPeerMessage(int serialID, IgpVirtualPeerMessageBox<T> &owner, int igpPeerIdent) : IgpMessage<T>(serialID, igpPeerIdent), owner(owner)
{
}

template <typename T>
IgpVirtualPeerMessage<T>::IgpVirtualPeerMessage(const Buffer<char> serialized, IgpVirtualPeerMessageBox<T> &owner, int igpPeerIdent)
    : IgpMessage<T>(serialized, igpPeerIdent), owner(owner)
{
}

template <typename T>
void IgpVirtualPeerMessage<T>::send()
{
    VoidBuffer out = T::serialize();
    owner.sendBuffer(out, isReliable(), IgpVirtualPeerMessage<T>::igpPeerIdent);
}

// IgpVirtualPeerMessageBox

template <typename T>
IgpVirtualPeerMessageBox<T>::IgpVirtualPeerMessageBox(IgpMessageListener &igpListener, int igpIdent)
    : IgpVirtualPeer(&igpListener, igpIdent), sendSerialID(0), m_isCorrupted(false)
{
}

template <typename T>
IgpVirtualPeerMessageBox<T>::IgpVirtualPeerMessageBox(IgpMessageListener &igpListener)
    : IgpVirtualPeer(&igpListener), sendSerialID(0), m_isCorrupted(false)
{
}

// Implement MessageBox
template <typename T>
Message * IgpVirtualPeerMessageBox<T>::createMessage()
{
    return new IgpVirtualPeerMessage<T>(++sendSerialID, *this, destIdent);
}

// Implement IgpPeer
template <typename T>
void IgpVirtualPeerMessageBox<T>::messageReceived(VoidBuffer message, int origIgpIdent, bool reliable)
{
    if (m_isCorrupted) return;
    try {
        IgpVirtualPeerMessage<T> incomingMessage(message, *this, origIgpIdent);
        propagateMessageToListeners(incomingMessage);
    }
    catch (Exception e) {
        fprintf(stderr, "Exception occured. We consider that the peer is corrupted!\n");
        e.printMessage();
        m_isCorrupted = true;
    }
}

// Own member functions
template <typename T>
void IgpVirtualPeerMessageBox<T>::sendBuffer(VoidBuffer out, bool reliable, int igpDestIdent)
{
    if (m_isCorrupted) return;
    sendMessageToAddress(out, igpDestIdent, reliable);
}

template <typename T>
void IgpVirtualPeerMessageBox<T>::bind(PeerAddress addr)
{
    if (m_isCorrupted) return;
    IgpPeerAddressImpl *peerAddressImpl = dynamic_cast<IgpPeerAddressImpl *>(addr.getImpl());
    if (peerAddressImpl != NULL) {
        destIdent = peerAddressImpl->getIgpIdent();
    }
    else throw Exception("Incompatible peer address type!");
}

}

#endif // _IGPVIRTUALPEERMESSAGEBOX_H

