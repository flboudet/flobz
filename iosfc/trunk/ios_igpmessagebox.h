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


#ifndef _IOS_IGPMESSAGEBOX_H
#define _IOS_IGPMESSAGEBOX_H

#include "ios_messagebox.h"
#include "ios_igpclient.h"
#include "ios_dirigeable.h"

namespace ios_fc {

// IGP stands for Ios Gateway Protocol

class IgpMessageBox : public MessageBox, IGPClientMessageListener {
public:
    IgpMessageBox(const String hostName, int portID);
    IgpMessageBox(const String hostName, int portID, int igpIdent);
    IgpMessageBox(MessageBox &mbox);
    IgpMessageBox(MessageBox &mbox, int igpIdent);
    virtual ~IgpMessageBox();
    virtual void idle();
    virtual Message * createMessage();
    void sendBuffer(VoidBuffer out, bool reliable, int igpDestIdent);
    void onMessage(VoidBuffer message, int origIdent, int destIdent);
    void bind(int igpIdent) { destIdent = igpIdent; }
    void bind(PeerAddress addr); // a revoir
    int getBound() const { return destIdent; }
 private:
    MessageBox *mbox;
    bool ownMessageBox;
    IGPClient igpClient;
    int sendSerialID;
    int destIdent;
};

}

#endif // _IOS_IGPMESSAGE_BOX_H
