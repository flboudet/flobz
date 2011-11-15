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
    InternetGameCenter(const String hostName, int portNum, const String name, const String password);
    void sendMessage(const String msgText);
    void idle();
    void onMessage(Message &msg);
    void setStatus(int status);
    String getSelfName();
    String getOpponentName();
    bool isConnected() const;
    bool isAccepted() const;
    bool isDenied() const;
    String getDenyString() const { return m_denyString; }
    String getDenyStringMore() const { return m_denyStringMore; }
    virtual FPServerIGPMessageBox *getIgpBox() { return m_igpmbox.get(); }
    // Temporaire
    void punch();
protected:
    void sendGameRequest(FloboGameInvitation &invitation);
    void sendGameAcceptInvitation(FloboGameInvitation &invitation);
    void sendGameCancelInvitation(FloboGameInvitation &invitation);
private:
    void sendAliveMessage();
    void grantGameToMBox(MessageBox &thembox);

    static const int fpipVersion;
    const String hostName;
    int portNum;
    std::auto_ptr<DatagramSocket>          m_udpSocket;
    std::auto_ptr<FPServerMessageBox>      m_udpmbox;
    std::auto_ptr<FPServerIGPMessageBox>   m_igpmbox;
    std::auto_ptr<DatagramSocket>          m_p2pSocket;
    std::auto_ptr<FPInternetP2PMessageBox> m_p2pmbox;
    std::auto_ptr<NatTraversal>            m_p2pNatTraversal;
    String p2pPunchName;
    bool tryNatTraversal;
    const String name;
    const String password;
    int status;
    double timeMsBetweenTwoAliveMessages, lastAliveMessage;
    FloboGameInvitation grantedInvitation;
    enum GameStatus {
        GAMESTATUS_IDLE,
        GAMESTATUS_STARTTRAVERSAL,
        GAMESTATUS_WAITTRAVERSAL,
        GAMESTATUS_GRANTED_P2P,
        GAMESTATUS_GRANTED_IGP
    };
    GameStatus gameGrantedStatus;
    String opponentName;
    bool m_isAccepted, m_isDenied;
    String m_denyString;
    String m_denyStringMore;
};

#endif // _PUYOINTERNETGAMECENTER_H

