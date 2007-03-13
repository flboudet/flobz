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
#include "ios_udpmessagebox.h"
#include "ios_igpmessage.h"

namespace ios_fc {

    class IgpMessageBoxMessage : public IgpMessage {
    public:
        IgpMessageBoxMessage(int serialID, IgpMessageBox &owner, int igpPeerIdent);
        IgpMessageBoxMessage(const Buffer<char> serialized, IgpMessageBox &owner, int igpPeerIdent) throw(InvalidMessageException);
        void sendBuffer(Buffer<char> out) const;
    private:
        IgpMessageBox &owner;
    };
    
    IgpMessageBoxMessage::IgpMessageBoxMessage(int serialID, IgpMessageBox &owner, int igpPeerIdent) : IgpMessage(serialID, igpPeerIdent), owner(owner)
    {
    }

    IgpMessageBoxMessage::IgpMessageBoxMessage(const Buffer<char> serialized, IgpMessageBox &owner, int igpPeerIdent) throw(InvalidMessageException)
    : IgpMessage(serialized, igpPeerIdent), owner(owner)
    {
    }
  
    void IgpMessageBoxMessage::sendBuffer(Buffer<char> out) const
    {
        owner.sendBuffer(out, isReliable(), igpPeerIdent);
    }
    
    // IgpMessageBox implementation
    IgpMessageBox::IgpMessageBox(const String hostName, int portID) : mbox(new UDPMessageBox(hostName, 0, portID)), ownMessageBox(true), igpClient(new IGPClient(*mbox)), sendSerialID(0)
    {
        igpClient->addListener(this);
    }
    
    IgpMessageBox::IgpMessageBox(const String hostName, int portID, int igpIdent) : mbox(new UDPMessageBox(hostName, 0, portID)), ownMessageBox(true), igpClient(new IGPClient(*mbox, igpIdent)), sendSerialID(0)
    {
        igpClient->addListener(this);
    }

    IgpMessageBox::IgpMessageBox(MessageBox &mbox) : mbox(&mbox), ownMessageBox(false), igpClient(new IGPClient(mbox)), sendSerialID(0)
    {
        igpClient->addListener(this);
    }

    IgpMessageBox::IgpMessageBox(MessageBox &mbox, int igpIdent) : mbox(&mbox), ownMessageBox(false), igpClient(new IGPClient(mbox, igpIdent)), sendSerialID(0)
    {
        igpClient->addListener(this);
    }

    IgpMessageBox::~IgpMessageBox()
    {
      delete igpClient;
      if (ownMessageBox)
	delete mbox;
    }

    void IgpMessageBox::idle()
    {
        igpClient->idle();
    }

    Message * IgpMessageBox::createMessage()
    {
        return new IgpMessageBoxMessage(++sendSerialID, *this, destIdent);
    }

    void IgpMessageBox::sendBuffer(VoidBuffer out, bool reliable, int igpDestIdent)
    {
        igpClient->sendMessage(igpDestIdent, out, reliable);
    }
    
    void IgpMessageBox::onMessage(VoidBuffer message, int origIdent, int destIdent)
    {
        try {
            IgpMessageBoxMessage incomingMessage(message, *this, origIdent);
            for (int i = 0, j = listeners.size() ; i < j ; i++) {
                MessageListener *currentListener = listeners[i];
                currentListener->onMessage(incomingMessage);
            }
        }
        catch (Exception e) {
            e.printMessage();
        }
    }
    
    void IgpMessageBox::bind(PeerAddress addr)
    {
        IgpMessage::IgpPeerAddressImpl *peerAddressImpl = dynamic_cast<IgpMessage::IgpPeerAddressImpl *>(addr.getImpl());
        if (peerAddressImpl != NULL) {
            destIdent = peerAddressImpl->getIgpIdent();
        }
        else throw Exception("Incompatible peer address type!");
    }
    
    PeerAddress IgpMessageBox::getSelfAddress() const
    {
        return PeerAddress(new IgpMessage::IgpPeerAddressImpl(igpClient->getIgpIdent()));
    }
}

