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

#ifndef _PUYOINTERNETGAMECENTER_H
#define _PUYOINTERNETGAMECENTER_H

#include <memory>
#include "NetworkDefinitions.h"
#include "NetGameCenter.h"
#include "NatTraversal.h"

class InternetGameCenter : public NetGameCenter, public MessageListener {
public:
    InternetGameCenter(const std::string &hostName, int portNum, const std::string &name, const std::string &password);
    void sendMessage(const std::string &msgText);
    void idle();
    void onMessage(Message &msg);
    void setStatus(int status);
    const std::string & getSelfName();
    const std::string & getOpponentName();
    bool isConnected() const;
    bool isAccepted() const;
    bool isDenied() const;
    std::string getDenyString() const { return _denyString; }
    std::string getDenyStringMore() const { return _denyStringMore; }
    virtual FPServerIGPMessageBox *getIgpBox() { return _igpmbox.get(); }
    // Temporaire
    void punch();
protected:
    void sendGameRequest(FloboGameInvitation &invitation);
    void sendGameAcceptInvitation(FloboGameInvitation &invitation);
    void sendGameCancelInvitation(FloboGameInvitation &invitation);
private:
    void sendAliveMessage();
    
    static const int fpipVersion;
    std::string _hostName;
    int _portNum;
    std::unique_ptr<DatagramSocket>          _udpSocket;
    std::unique_ptr<FPServerMessageBox>      _udpmbox;
    std::unique_ptr<FPServerIGPMessageBox>   _igpmbox;
    std::unique_ptr<DatagramSocket>          _p2pSocket;
    std::unique_ptr<FPInternetP2PMessageBox> _p2pmbox;
    std::unique_ptr<NatTraversal>            _p2pNatTraversal;
    std::string _p2pPunchName;
    bool _tryNatTraversal;
    std::string _name;
    std::string _password;
    int _status;
    double _timeMsBetweenTwoAliveMessages, _lastAliveMessage;
    FloboGameInvitation _grantedInvitation;
    enum GameStatus {
        GAMESTATUS_IDLE,
        GAMESTATUS_STARTTRAVERSAL,
        GAMESTATUS_WAITTRAVERSAL,
        GAMESTATUS_GRANTED_P2P,
        GAMESTATUS_GRANTED_IGP
    };
    GameStatus _gameGrantedStatus;
    std::string _opponentName;
    bool _isAccepted, _isDenied;
    std::string _denyString;
    std::string _denyStringMore;
};

#endif // _PUYOINTERNETGAMECENTER_H

