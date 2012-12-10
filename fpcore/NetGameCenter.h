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

#ifndef _PUYONETGAMECENTER_H
#define _PUYONETGAMECENTER_H

#include "ios_memory.h"
#include "ios_dirigeable.h"
#include "ios_messagebox.h"
#include "NetworkDefinitions.h"

using namespace ios_fc;

class FloboGameInvitation {
public:
    FloboGameInvitation();
public:
    PeerAddress initiatorAddress;
    String opponentName;
    PeerAddress opponentAddress;
    unsigned long gameRandomSeed;
    int gameSpeed;
    int gameNbSets;
};

class NetGameCenterListener {
public:
    virtual void onChatMessage(const String &msgAuthor, const String &msg) = 0;
    virtual void onPlayerConnect(String playerName, PeerAddress playerAddress) = 0;
    virtual void onPlayerDisconnect(String playerName, PeerAddress playerAddress) = 0;
    virtual void onPlayerUpdated(String playerName, PeerAddress playerAddress) = 0;
    virtual void onGameInvitationReceived(FloboGameInvitation &invitation) = 0;
    virtual void onGameInvitationCanceledReceived(FloboGameInvitation &invitation) = 0;
    virtual void onGameAcceptedNegociationPending(FloboGameInvitation &invitation) {}
    virtual void onGameGrantedWithMessagebox(MessageBox *mbox, FloboGameInvitation &invitation) = 0;
    virtual ~NetGameCenterListener() {};
};

enum PeerStatus {
  PEER_NORMAL = 0,
  PEER_PLAYING = 1,
};

struct PeerInfo {
    int status;
    int rank;
    bool self;
};

class NetGameCenter {
public:
    NetGameCenter() : pendingGameTimeout(30000.) {}
    virtual ~NetGameCenter() {}
    virtual void sendMessage(const String msgText) = 0;
    // Request a new game
    void requestGame(FloboGameInvitation &invitation);
    // Accept a game invitation
    void acceptGameInvitation(FloboGameInvitation &invitation);
    // Cancel a game invitation
    void cancelGameInvitation(FloboGameInvitation &invitation);
    virtual void idle();
    String getPeerNameAtIndex(int i) const;
    PeerAddress getPeerAddressAtIndex(int i) const;
    PeerAddress getPeerAddressForPeerName(String peerName) const;
    PeerInfo getPeerInfoForAddress(PeerAddress &addr) const;
    int getPeerStatusForAddress(PeerAddress &addr) const;
    int getPeerCount() const;
    void addListener(NetGameCenterListener *r) { listeners.add(r); }
    void removeListener(NetGameCenterListener *r) { listeners.remove(r); }
    void connectPeer(PeerAddress addr, const String name, int status = PEER_NORMAL, int rank = -1, bool self = false);
    void disconnectPeer(PeerAddress addr, const String name);
    virtual void setStatus(int status) {}
    virtual String getSelfName() { return "Myself"; }
    virtual String getOpponentName() { return "Opponent"; }
    // Get the undelying message box (default: NULL)
    virtual MessageBox *getMessageBox() { return NULL; }
    // Optional server managing the game center.
    virtual FPServerIGPMessageBox *getIgpBox() { return NULL; }
protected:
    AdvancedBuffer<NetGameCenterListener *> listeners;
    class GamerPeer;
    AdvancedBuffer<GamerPeer *> peers;
    class PendingGame;
    AdvancedBuffer<PendingGame *> pendingGames;
    GamerPeer *getPeerForAddress(PeerAddress addr);
    // Should be called by implementations when an invitation is received
    void receivedGameInvitation(FloboGameInvitation &invitation);
    void receivedGameCanceledWithPeer(String playerName, PeerAddress addr);
    // Implement the sending of the game request
    virtual void sendGameRequest(FloboGameInvitation &invitation) = 0;
    // Implement the sending of the acceptation of a game
    virtual void sendGameAcceptInvitation(FloboGameInvitation &invitation) = 0;
    // Implement the sending of the cancelation of a game
    virtual void sendGameCancelInvitation(FloboGameInvitation &invitation) = 0;
    // Grant the game with a message box
    void grantGameWithMessageBox(FloboGameInvitation &invitation, MessageBox &thembox);
    double pendingGameTimeout;
};

#endif // _PUYONETGAMECENTER_H
