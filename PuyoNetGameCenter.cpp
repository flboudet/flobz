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
 

#include "PuyoNetGameCenter.h"
#include "ios_time.h"

using namespace ios_fc;

class PuyoNetGameCenter::GamerPeer {
public:
    GamerPeer(String name, PeerAddress address) : name(name), address(address) {}
    String name;
    PeerAddress address;
};

class PuyoNetGameCenter::PendingGame {
public:
    PendingGame(GamerPeer *peer) : peer(peer), initiateTime(getTimeMs()) {}
    GamerPeer *peer;
    double initiateTime;
};

void PuyoNetGameCenter::idle()
{
    double time_ms = getTimeMs();
    for (int i = pendingGames.size() - 1 ; i >= 0 ; i--) {
        if (time_ms - pendingGames[i]->initiateTime > pendingGameTimeout) {
            for (int u = 0, v = listeners.size() ; u < v ; u++) {
                listeners[u]->gameCanceledAgainst(pendingGames[i]->peer->name, pendingGames[i]->peer->address);
            }
            cancelGameWithPeer(pendingGames[i]->peer->name, pendingGames[i]->peer->address);
            delete pendingGames[i];
            pendingGames.removeAt(i);
        }
    }
}

void PuyoNetGameCenter::connectPeer(PeerAddress addr, const String name)
{
    //printf("%s vient de se connecter!\n", (const char *)name);
    for (int i = 0, j = peers.size() ; i < j ; i++) {
        if (peers[i]->address == addr)
            return;
    }
    peers.add(new GamerPeer(name, addr));
    for (int i = 0, j = listeners.size() ; i < j ; i++) {
        listeners[i]->onPlayerConnect(name, addr);
    }
}

void PuyoNetGameCenter::disconnectPeer(PeerAddress addr, const String name)
{
    //printf("%s vient de se deconnecter!\n", (const char *)name);
    for (int i = 0, j = peers.size() ; i < j ; i++) {
        GamerPeer *currentPeer = peers[i];
        if (currentPeer->address == addr) {
            // Cancels all games from this peer
            for (int i = pendingGames.size() - 1 ; i >= 0 ; i--) {
                if (pendingGames[i]->peer == currentPeer) {
                    // We won't call gameCanceledAgainst yet to avoid objects destructing themselves
                    // but instead set the game in timeout
               	    pendingGames[i]->initiateTime = getTimeMs() - pendingGameTimeout;
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
}

String PuyoNetGameCenter::getPeerNameAtIndex(int i) const
{
    return peers[i]->name;
}

PeerAddress PuyoNetGameCenter::getPeerAddressAtIndex(int i) const
{
    return peers[i]->address;
}

PeerAddress PuyoNetGameCenter::getPeerAddressForPeerName(String peerName) const
{
    int i;
    for (i = 0 ; i < peers.size() ; i++) {
        if (peers[i]->name == peerName)
            return peers[i]->address;
    }
    // hum...
    return peers[i]->address;
}

int PuyoNetGameCenter::getPeerCount() const
{
    return peers.size();
}

void PuyoNetGameCenter::requestGameWith(PeerAddress addr)
{
    GamerPeer *myPeer = getPeerForAddress(addr);
    if (myPeer != NULL) {
        // Checks if there is already an active game with this peer
        for (int i = pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (pendingGames[i]->peer == myPeer)
                return;
        }
        pendingGames.add(new PendingGame(myPeer));
        requestGameWithPeer(myPeer->name, myPeer->address);
    }
}

void PuyoNetGameCenter::cancelGameWith(PeerAddress addr)
{
    GamerPeer *myPeer = getPeerForAddress(addr);
    if (myPeer != NULL) {
        for (int i = pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (pendingGames[i]->peer == myPeer) {
                for (int u = 0, v = listeners.size() ; u < v ; u++) {
                    listeners[u]->gameCanceledAgainst(pendingGames[i]->peer->name, pendingGames[i]->peer->address);
                }
                cancelGameWithPeer(pendingGames[i]->peer->name, pendingGames[i]->peer->address);
                delete pendingGames[i];
                pendingGames.removeAt(i);
            }
        }
    }
}

void PuyoNetGameCenter::receivedInvitationForGameWithPeer(String playerName, PeerAddress addr)
{
    // Retrieving the peer
    GamerPeer *peer = getPeerForAddress(addr);
    if (peer != NULL) {
        // Check if the game doesn't exists
        for (int i = pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (pendingGames[i]->peer == peer)
                return;
        }
        // Create the game
        pendingGames.add(new PendingGame(peer));
        // notifies invitation
        for (int i = 0, j = listeners.size() ; i < j ; i++) {
            listeners[i]->gameInvitationAgainst(peer->name, peer->address);
        }
    }
}

void PuyoNetGameCenter::receivedGameCanceledWithPeer(String playerName, PeerAddress addr)
{
    // Retrieving the peer
    GamerPeer *peer = getPeerForAddress(addr);
    if (peer != NULL) {
        // Check if the game doesn't exists
        for (int i = pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (pendingGames[i]->peer == peer) {
                // Cancel the game
                for (int u = 0, v = listeners.size() ; u < v ; u++) {
                    listeners[u]->gameCanceledAgainst(pendingGames[i]->peer->name, pendingGames[i]->peer->address);
                }
                delete pendingGames[i];
                pendingGames.removeAt(i);
            }
        }
    }
}

void PuyoNetGameCenter::acceptInvitationWith(PeerAddress addr)
{
    GamerPeer *myPeer = getPeerForAddress(addr);
    acceptInvitationWithPeer(myPeer->name, myPeer->address);
}

PuyoNetGameCenter::GamerPeer *PuyoNetGameCenter::getPeerForAddress(PeerAddress addr)
{
    for (int i = 0 ; i < peers.size() ; i++) {
        if (peers[i]->address == addr) {
            return peers[i];
        }
    }
    return NULL;
}

