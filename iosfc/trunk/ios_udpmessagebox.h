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

#ifndef _IOS_UDP_MESSAGE_BOX_H
#define _IOS_UDP_MESSAGE_BOX_H

#include "ios_memory.h"
#include "ios_messagebox.h"
#include "IosDatagramSocket.h"

namespace ios_fc {

class UDPMessageBox;
class UDPRawMessage;

class UDPMessageBox : public MessageBox {
public:
    UDPMessageBox(char *address, int localPort, int remotePort);
    virtual ~UDPMessageBox() {}
    void idle();
    Message * createMessage();
    void sendUDP(Buffer<char> buffer, int id, bool reliable);
private:
    IosDatagramSocket socket;
    int sendSerialID, receiveSerialID;
    int cyclesBeforeResendingReliable;
    int waitingForAckTimeout;
    UDPRawMessage *waitingForAckMessage;
    Vector<UDPRawMessage*> outQueue;
    void sendQueue();
};

};

#endif // _IOS_UDP_MESSAGE_BOX_H
