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

    IgpMessage::IgpMessage(const Buffer<char> serialized, IgpMessageBox &owner) throw(InvalidMessageException)
    : StandardMessage(serialized), owner(owner)
    {
    }
    
    IgpMessage::~IgpMessage()
    {
    }
  
    void IgpMessage::sendBuffer(Buffer<char> out) const
    {
        owner.sendBuffer(out);
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
        return new IgpMessage(++sendSerialID, *this);
    }

    void IgpMessageBox::sendBuffer(VoidBuffer out)
    {
        igpClient.sendMessage(25, out);
    }
    
    void IgpMessageBox::onMessage(VoidBuffer message, int origIdent, int destIdent)
    {
        try {
            IgpMessage incomingMessage(message, *this);
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

