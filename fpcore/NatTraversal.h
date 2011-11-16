/* FloboPop
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gyom@ios-software.com>
 *
 * iOS Software <http://www.ios-software.com>
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

#ifndef _PUYONATTRAVERSAL_H
#define _PUYONATTRAVERSAL_H

#include "ios_udpmessagebox.h"
#include "ios_igpmessagebox.h"
#include "ios_udpmessage.h"
#include "ios_time.h"
#include "FPIGPDefs.h"
#include <unistd.h>

using namespace ios_fc;

class NatTraversal : public MessageListener {
public:
    NatTraversal(UDPMessageBoxBase &udpmbox, double punchInfoTimeout = 3000., double strategyTimeout = 2000.);
    virtual ~NatTraversal();
    void punch(const String punchPoolName);
    void idle();
    void onMessage(Message &message);
    inline bool hasFailed() { return (currentStrategy == FAILED); }
    inline bool hasSucceeded() { return (currentStrategy == SUCCESS); }
private:
    void sendGarbageMessage();
    void sendSyncMessage();

    UDPMessageBoxBase &udpmbox;
    IgpMessageBoxBase *igpmbox;
    String peerAddressString, peerLocalAddressString;
    int peerPortNum, peerLocalPortNum;
    enum {
        TRY_NONE = 0,
        TRY_PUBLICADDR = 1,
        TRY_PUBLICADDR_NEXTPORT = 2,
        TRY_LOCALADDR = 3,
        FAILED = 4,
        SYNCING = 5,
        SUCCESS = 6
    };
    int currentStrategy;
    double punchInfoTimeout, strategyTimeout, timeToPunchInfo, timeToNextStrategy;
    int receivedGarbage;
    bool gettingPunchInfo;
    String punchPoolName;
    PeerAddress udpPeerAddress;
    SocketAddress udpSocketAddress;
    int udpSocketPortNum;
    SocketAddress igpServerSocketAddress;
    int igpServerPortNum;
};

#endif // _PUYONATTRAVERSAL_H

