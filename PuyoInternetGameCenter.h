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

#ifndef _PUYOINTERNETGAMECENTER_H
#define _PUYOINTERNETGAMECENTER_H

#include "ios_igpmessagebox.h"
#include "ios_udpmessagebox.h"
#include "PuyoNetGameCenter.h"
#include "PuyoNatTraversal.h"

using namespace ios_fc;

class PuyoInternetGameCenter : public PuyoNetGameCenter, public MessageListener {
public:
    PuyoInternetGameCenter(const String hostName, int portNum, const String name, const String password);
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
    // Temporaire
    void punch();
protected:
    void sendGameRequest(PuyoGameInvitation &invitation);
    void sendGameAcceptInvitation(PuyoGameInvitation &invitation);
    void sendGameCancelInvitation(PuyoGameInvitation &invitation);
private:
    void sendAliveMessage();
    void grantGameToMBox(MessageBox &thembox);
    
    static const int fpipVersion;
    const String hostName;
    int portNum;
    IgpMessageBox mbox;
    UDPMessageBox *p2pmbox;
    PuyoNatTraversal *p2pNatTraversal;
    String p2pPunchName;
    bool tryNatTraversal;
    const String name;
    const String password;
    int status;
    double timeMsBetweenTwoAliveMessages, lastAliveMessage;
    PuyoGameInvitation grantedInvitation;
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

