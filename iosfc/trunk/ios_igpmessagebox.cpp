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

namespace ios_fc {

    class IgpMessage : public StandardMessage {
    public:
        IgpMessage(int serialID, IgpMessageBox &owner);
        IgpMessage(const Buffer<char> serialized, IgpMessageBox &owner) throw(InvalidMessageException);
        virtual ~IgpMessage();
        void sendBuffer(Buffer<char> out) const;
        
    private:
        IgpMessageBox &owner;
    };

    IgpMessage::IgpMessage(int serialID, IgpMessageBox &owner) : StandardMessage(serialID), owner(owner)
    {
    }

    IgpMessage::~IgpMessage()
    {
    }
  
    void IgpMessage::sendBuffer(Buffer<char> out) const
    {
        owner.sendBuffer(out);
    }
    
    
    IgpMessageBox::IgpMessageBox(const String hostName, int portID)
    {
        sock = new Socket(hostName, portID);
        sendSerialID = 0;
    }

    IgpMessageBox::~IgpMessageBox()
    {
        delete sock;
    }

    void IgpMessageBox::idle()
    {
    }

    Message * IgpMessageBox::createMessage()
    {
        return new IgpMessage(++sendSerialID, *this);
    }

    void IgpMessageBox::sendBuffer(Buffer<char> out) const
    {
        sock->getOutputStream()->streamWrite(out);
    }
    
};

