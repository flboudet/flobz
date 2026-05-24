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

#ifndef _PUYOLANGAMECENTER_H
#define _PUYOLANGAMECENTER_H

#include "NetworkDefinitions.h"
#include "NetGameCenter.h"
#include "ios_networkinterfacerequester.h"

class LanGameCenter : public NetGameCenter, public MessageListener, public SessionListener, NetGameCenterListener {
public:
    LanGameCenter(int portNum, const std::string & name);
    virtual ~LanGameCenter();
    void sendMessage(const std::string & msgText);
    void idle();
    void setStatus(int status);
    void onMessage(Message &msg);
    void onPeerConnect(const PeerAddress &address) {}
    void onPeerDisconnect(const PeerAddress &address);
    const std::string & getSelfName();
    const std::string & getOpponentName();
// NetGameCenterListener implementation
    virtual void onChatMessage(const std::string &msgAuthor, const std::string &msg) {}
    virtual void onPlayerConnect(const std::string & playerName, PeerAddress playerAddress);
    virtual void onPlayerDisconnect(const std::string & playerName, PeerAddress playerAddress) {}
    virtual void onPlayerUpdated(const std::string & playerName, PeerAddress playerAddress) {}
    virtual void onGameInvitationReceived(FloboGameInvitation &invitation) {}
    virtual void onGameInvitationCanceledReceived(FloboGameInvitation &invitation) {}
    virtual void onGameGrantedWithMessagebox(MessageBox *mbox, FloboGameInvitation &invitation) {}
protected:
    void sendGameRequest(FloboGameInvitation &invitation);
    void sendGameAcceptInvitation(FloboGameInvitation &invitation);
    void sendGameCancelInvitation(FloboGameInvitation &invitation);
private:
    void sendAliveMessage();
    void sendDisconnectMessage();
    void grantGame(FloboGameInvitation &invitation);
    DatagramSocket _socket;
    FPLANMessageBox _mbox;
    std::string _name;
    int _uuid;
    double _timeMsBetweenTwoAliveMessages, _lastAliveMessage;
    double _timeMsBetweenTwoNetworkInterfacesDetection, _lastNetworkInterfacesDetection;
    bool _gameGranted;
    FloboGameInvitation _grantedInvitation;
    int _status;
    std::string _opponentName;

    SocketAddress _multicastAddress, _loopbackAddress;
    NetworkInterfaceRequester _requester;
    std::vector<NetworkInterface> _networkInterfaces;
    UDPPeerAddress _mcastPeerAddress;
};

#endif // _PUYOLANGAMECENTER_H

