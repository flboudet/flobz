/* FloboPuyo
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

#ifndef _PUYOLANGAMECENTER_H
#define _PUYOLANGAMECENTER_H

#include "ios_datagramsocket.h"
#include "ios_udpmessagebox.h"
#include "PuyoNetGameCenter.h"

using namespace ios_fc;

class PuyoLanGameCenter : public PuyoNetGameCenter, public MessageListener, public SessionListener {
public:
    PuyoLanGameCenter(int portNum, const String name);
    void sendMessage(const String msgText);
    void idle();
    void setStatus(int status);
    void onMessage(Message &msg);
    void onPeerConnect(const PeerAddress &address) {}
    void onPeerDisconnect(const PeerAddress &address);
    String getSelfName();
    String getOpponentName();
protected:
    void requestGameWithPeer(String playerName, PeerAddress addr);
    void acceptInvitationWithPeer(String playerName, PeerAddress addr);
    void cancelGameWithPeer(String playerName, PeerAddress addr);
private:
    void sendAliveMessage();
    void grantGameToPeer(PeerAddress addr);
    DatagramSocket socket;
    UDPMessageBox mbox;
    const String name;
    double timeMsBetweenTwoAliveMessages, lastAliveMessage;
    bool gameGranted;
    PeerAddress grantedAddr;
    int status;
    String opponentName;
};

#endif // _PUYOLANGAMECENTER_H

