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

#include "ios_fastmessage.h"
#include "ios_igpmessagebox.h"

namespace ios_fc {

    // IgpMessageBoxBase implementation
    IgpMessageBoxBase::IgpMessageBoxBase(MessageBox *mbox) : mbox(mbox), ownMessageBox(false), igpClient(new IGPClient(*mbox)), sendSerialID(0)
    {
        igpClient->addListener(this);
    }

    IgpMessageBoxBase::IgpMessageBoxBase(MessageBox *mbox, int igpIdent) : mbox(mbox), ownMessageBox(false), igpClient(new IGPClient(*mbox, igpIdent)), sendSerialID(0)
    {
        igpClient->addListener(this);
    }

    IgpMessageBoxBase::~IgpMessageBoxBase()
    {
      delete igpClient;
      if (ownMessageBox)
          delete mbox;
    }

    void IgpMessageBoxBase::idle()
    {
        igpClient->idle();
    }

    void IgpMessageBoxBase::sendBuffer(VoidBuffer out, bool reliable, int igpDestIdent)
    {
        igpClient->sendMessage(igpDestIdent, out, reliable);
    }

    void IgpMessageBoxBase::bind(PeerAddress addr)
    {
        IgpPeerAddressImpl *peerAddressImpl = dynamic_cast<IgpPeerAddressImpl *>(addr.getImpl());
        if (peerAddressImpl != NULL) {
            destIdent = peerAddressImpl->getIgpIdent();
        }
        else throw Exception("Incompatible peer address type!");
    }

    PeerAddress IgpMessageBoxBase::getSelfAddress() const
    {
        return PeerAddress(new IgpPeerAddressImpl(igpClient->getIgpIdent()));
    }
    
}

