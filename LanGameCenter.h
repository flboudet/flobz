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

#include "NetworkDefinitions.h"
#include "NetGameCenter.h"
#include "ios_networkinterfacerequester.h"

class LanGameCenter : public NetGameCenter, public MessageListener, public SessionListener, NetGameCenterListener {
public:
    LanGameCenter(int portNum, const String name);
    virtual ~LanGameCenter();
    void sendMessage(const String msgText);
    void idle();
    void setStatus(int status);
    void onMessage(Message &msg);
    void onPeerConnect(const PeerAddress &address) {}
    void onPeerDisconnect(const PeerAddress &address);
    String getSelfName();
    String getOpponentName();
// NetGameCenterListener implementation
    virtual void onChatMessage(const String &msgAuthor, const String &msg) {}
    virtual void onPlayerConnect(String playerName, PeerAddress playerAddress);
    virtual void onPlayerDisconnect(String playerName, PeerAddress playerAddress) {}
    virtual void onPlayerUpdated(String playerName, PeerAddress playerAddress) {}
    virtual void onGameInvitationReceived(PuyoGameInvitation &invitation) {}
    virtual void onGameInvitationCanceledReceived(PuyoGameInvitation &invitation) {}
    virtual void onGameGrantedWithMessagebox(MessageBox *mbox, PuyoGameInvitation &invitation) {}
protected:
    void sendGameRequest(PuyoGameInvitation &invitation);
    void sendGameAcceptInvitation(PuyoGameInvitation &invitation);
    void sendGameCancelInvitation(PuyoGameInvitation &invitation);
private:
    void sendAliveMessage();
    void sendDisconnectMessage();
    void grantGame(PuyoGameInvitation &invitation);
    DatagramSocket socket;
    FPLANMessageBox mbox;
    const String name;
    int m_uuid;
    double timeMsBetweenTwoAliveMessages, lastAliveMessage;
    double timeMsBetweenTwoNetworkInterfacesDetection, lastNetworkInterfacesDetection;
    bool gameGranted;
    PuyoGameInvitation grantedInvitation;
    int status;
    String opponentName;

    SocketAddress multicastAddress, loopbackAddress;
    NetworkInterfaceRequester requester;
    std::vector<NetworkInterface> networkInterfaces;
    UDPPeerAddress mcastPeerAddress;
};

#endif // _PUYOLANGAMECENTER_H

