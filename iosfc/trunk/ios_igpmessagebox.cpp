/* Ultimate Othello 1678
 * Copyright (C) 2002  Florent Boudet <flobo@ifrance.com>
 * iOS Software <http://ios.free.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 */

#include "ios_igpmessagebox.h"
#include "ios_standardmessage.h"
#include "ios_dirigeable.h"

namespace ios_fc {

    class IgpMessage : public StandardMessage, public Dirigeable {
    public:
        IgpMessage(int serialID, IgpMessageBox &owner, int igpPeerIdent);
        IgpMessage(const Buffer<char> serialized, IgpMessageBox &owner, int igpPeerIdent) throw(InvalidMessageException);
        virtual ~IgpMessage();
        void sendBuffer(Buffer<char> out) const;
        // Dirigeable
        PeerAddress getPeerAddress();
        PeerAddress getBroadcastAddress();
        void setPeerAddress(PeerAddress);
    private:
        class IgpPeerAddressImpl;
        IgpMessageBox &owner;
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
    
    IgpMessage::IgpMessage(int serialID, IgpMessageBox &owner, int igpPeerIdent) : StandardMessage(serialID), owner(owner), igpPeerIdent(igpPeerIdent)
    {
    }

    IgpMessage::IgpMessage(const Buffer<char> serialized, IgpMessageBox &owner, int igpPeerIdent) throw(InvalidMessageException)
    : StandardMessage(serialized), owner(owner), igpPeerIdent(igpPeerIdent)
    {
    }
    
    IgpMessage::~IgpMessage()
    {
    }
  
    void IgpMessage::sendBuffer(Buffer<char> out) const
    {
        owner.sendBuffer(out, isReliable(), igpPeerIdent);
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
    
    IgpMessageBox::IgpMessageBox(const String hostName, int portID) : igpClient(hostName, portID), sendSerialID(0)
    {
        igpClient.addListener(this);
    }
    
    IgpMessageBox::IgpMessageBox(const String hostName, int portID, int igpIdent) : igpClient(hostName, portID, igpIdent), sendSerialID(0)
    {
        igpClient.addListener(this);
    }

    IgpMessageBox::~IgpMessageBox()
    {
    }

    void IgpMessageBox::idle()
    {
        igpClient.idle();
    }

    Message * IgpMessageBox::createMessage()
    {
        return new IgpMessage(++sendSerialID, *this, destIdent);
    }

    void IgpMessageBox::sendBuffer(VoidBuffer out, bool reliable, int igpDestIdent)
    {
        igpClient.sendMessage(igpDestIdent, out, reliable);
    }
    
    void IgpMessageBox::onMessage(VoidBuffer message, int origIdent, int destIdent)
    {
        try {
            IgpMessage incomingMessage(message, *this, origIdent);
            for (int i = 0, j = listeners.size() ; i < j ; i++) {
                MessageListener *currentListener = listeners[i];
                currentListener->onMessage(incomingMessage);
            }
        }
        catch (Exception e) {
            e.printMessage();
        }
    }
};

