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


#include "NetGameCenter.h"
#include "ios_time.h"

using namespace ios_fc;

class NetGameCenter::GamerPeer {
public:
    GamerPeer(String name, PeerAddress address, int status = PEER_NORMAL, int rank = -1, bool self=false) :
        name(name), address(address), status(status), rank(rank), self(self) {}
    String name;
    PeerAddress address;
    int status;
    int rank;
    bool self;
};

class NetGameCenter::PendingGame {
public:
    PendingGame(GamerPeer *peer, FloboGameInvitation &invitation) : peer(peer), invitation(invitation), initiateTime(getTimeMs()) {}
    GamerPeer *peer;
    FloboGameInvitation invitation;
    double initiateTime;
};

void NetGameCenter::idle()
{
    double time_ms = getTimeMs();
    for (int i = pendingGames.size() - 1 ; i >= 0 ; i--) {
        if (time_ms - pendingGames[i]->initiateTime > pendingGameTimeout) {
            for (int u = 0, v = listeners.size() ; u < v ; u++) {
                listeners[u]->onGameInvitationCanceledReceived(pendingGames[i]->invitation);
            }
            sendGameCancelInvitation(pendingGames[i]->invitation);
            delete pendingGames[i];
            pendingGames.removeAt(i);
        }
    }
}

void NetGameCenter::connectPeer(PeerAddress addr, const String name, int status, int rank, bool self)
{
    //printf("%s vient de se connecter!\n", (const char *)name);
    for (int i = 0, j = peers.size() ; i < j ; i++) {
        if (peers[i]->address == addr) {
            if (peers[i]->status != status) {
                peers[i]->status = status;
                peers[i]->rank = rank;
                peers[i]->self = self;
                for (int i = 0, j = listeners.size() ; i < j ; i++) {
                    listeners[i]->onPlayerUpdated(name, addr);
                }
            }
            return;
        }
    }
    // Peer is not known. Add to list
    peers.add(new GamerPeer(name, addr, status, rank, self));
    for (int i = 0, j = listeners.size() ; i < j ; i++) {
        listeners[i]->onPlayerConnect(name, addr);
    }
}

void NetGameCenter::disconnectPeer(PeerAddress addr, const String name)
{
    //printf("%s vient de se deconnecter! (parait il)\n", (const char *)name);
    for (int i = 0, j = peers.size() ; i < j ; i++) {
        GamerPeer *currentPeer = peers[i];
        if (currentPeer->address == addr) {
            //printf("Peer trouve au nom %s\n", (const char *)name);
            // Cancels all games from this peer
            for (int i = pendingGames.size() - 1 ; i >= 0 ; i--) {
                if (pendingGames[i]->peer == currentPeer) {
                    for (int u = 0, v = listeners.size() ; u < v ; u++) {
                        listeners[u]->onGameInvitationCanceledReceived(pendingGames[i]->invitation);
                    }
                    delete pendingGames[i];
                    pendingGames.removeAt(i);
                }
            }
            // Delete the disconnected peer
            peers.remove(currentPeer);
            for (int u = 0, v = listeners.size() ; u < v ; u++) {
                listeners[u]->onPlayerDisconnect(currentPeer->name, currentPeer->address);
            }
            delete currentPeer;
            return;
        }
    }
    //printf("Pas de peer trouve au nom %s\n", (const char *)name);
}

String NetGameCenter::getPeerNameAtIndex(int i) const
{
    return peers[i]->name;
}

PeerAddress NetGameCenter::getPeerAddressAtIndex(int i) const
{
    return peers[i]->address;
}

PeerAddress NetGameCenter::getPeerAddressForPeerName(String peerName) const
{
    int i;
    for (i = 0 ; i < peers.size() ; i++) {
        if (peers[i]->name == peerName)
            return peers[i]->address;
    }
    // hum...
    return peers[i]->address;
}

PeerInfo NetGameCenter::getPeerInfoForAddress(PeerAddress &addr) const
{
    PeerInfo info;
    info.status = 0;
    info.rank = -1;
    info.self = false;
    for (int i = 0 ; i < peers.size() ; i++) {
        if (peers[i]->address == addr) {
            info.status = peers[i]->status;
            info.rank = peers[i]->rank;
            info.self = peers[i]->self;
            return info;
        }
    }
    return info;
}

int NetGameCenter::getPeerStatusForAddress(PeerAddress &addr) const
{
    for (int i = 0 ; i < peers.size() ; i++) {
        if (peers[i]->address == addr)
            return peers[i]->status;
    }
    return 0;
}

int NetGameCenter::getPeerCount() const
{
    return peers.size();
}

void NetGameCenter::requestGame(FloboGameInvitation &invitation)
{
    GamerPeer *myPeer = getPeerForAddress(invitation.opponentAddress);
    if (myPeer != NULL) {
        // Checks if there is already an active game with this peer
        for (int i = pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (pendingGames[i]->peer == myPeer)
                return;
        }
        invitation.opponentName = myPeer->name;
        pendingGames.add(new PendingGame(myPeer, invitation));
        sendGameRequest(invitation);
    }
}

void NetGameCenter::acceptGameInvitation(FloboGameInvitation &invitation)
{
    GamerPeer *myPeer = getPeerForAddress(invitation.opponentAddress);
    if (myPeer != NULL)
        sendGameAcceptInvitation(invitation);
}

void NetGameCenter::cancelGameInvitation(FloboGameInvitation &invitation)
{
    GamerPeer *myPeer = getPeerForAddress(invitation.opponentAddress);
    if (myPeer != NULL) {
        for (int i = pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (pendingGames[i]->peer == myPeer) {
                // We won't call gameCanceledAgainst yet to avoid objects destructing themselves
                // but instead set the game in timeout
                pendingGames[i]->initiateTime = getTimeMs() - pendingGameTimeout;
            }
        }
    }
}

void NetGameCenter::receivedGameInvitation(FloboGameInvitation &invitation)
{
    // Retrieving the peer
    GamerPeer *peer = getPeerForAddress(invitation.opponentAddress);
    if (peer != NULL) {
        // Check if the game doesn't exists
        for (int i = pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (pendingGames[i]->peer == peer)
                return;
        }
        // Create the game
        pendingGames.add(new PendingGame(peer, invitation));
        // notifies invitation
        for (int i = 0, j = listeners.size() ; i < j ; i++) {
            listeners[i]->onGameInvitationReceived(invitation);
        }
    }
}

void NetGameCenter::receivedGameCanceledWithPeer(String playerName, PeerAddress addr)
{
    // Retrieving the peer
    GamerPeer *peer = getPeerForAddress(addr);
    if (peer != NULL) {
        // Check if the game doesn't exists
        for (int i = pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (pendingGames[i]->peer == peer) {
                // Cancel the game
                for (int u = 0, v = listeners.size() ; u < v ; u++) {
                    listeners[u]->onGameInvitationCanceledReceived(pendingGames[i]->invitation);
                }
                delete pendingGames[i];
                pendingGames.removeAt(i);
            }
        }
    }
}

NetGameCenter::GamerPeer *NetGameCenter::getPeerForAddress(PeerAddress addr)
{
    for (int i = 0 ; i < peers.size() ; i++) {
        if (peers[i]->address == addr) {
            return peers[i];
        }
    }
    return NULL;
}

