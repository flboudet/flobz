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

FloboGameInvitation::FloboGameInvitation()
    : _gameRandomSeed(0), _gameSpeed(0), _gameNbSets(0)
{}

class NetGameCenter::GamerPeer {
public:
    GamerPeer(const std::string & name, PeerAddress address, int status = PEER_NORMAL, int rank = -1, bool self=false) :
        _name(name), _address(address), _status(status), _rank(rank), _self(self) {}
    std::string _name;
    PeerAddress _address;
    int _status;
    int _rank;
    bool _self;
};

class NetGameCenter::PendingGame {
public:
    PendingGame(GamerPeer *peer, FloboGameInvitation &invitation) : _peer(peer), _invitation(invitation), _initiateTime(getTimeMs()) {}
    GamerPeer *_peer;
    FloboGameInvitation _invitation;
    double _initiateTime;
};

void NetGameCenter::idle()
{
    double time_ms = getTimeMs();
    for (int i = _pendingGames.size() - 1 ; i >= 0 ; i--) {
        if (time_ms - _pendingGames[i]->_initiateTime > _pendingGameTimeout) {
            for (int u = 0, v = _listeners.size() ; u < v ; u++) {
                _listeners[u]->onGameInvitationCanceledReceived(_pendingGames[i]->_invitation);
            }
            sendGameCancelInvitation(_pendingGames[i]->_invitation);
            delete _pendingGames[i];
            _pendingGames.removeAt(i);
        }
    }
}

void NetGameCenter::connectPeer(PeerAddress addr, const std::string &name, int status, int rank, bool self)
{
    //printf("%s vient de se connecter!\n", (const char *)name);
    for (int i = 0, j = _peers.size() ; i < j ; i++) {
        if (_peers[i]->_address == addr) {
            if (_peers[i]->_status != status) {
                _peers[i]->_status = status;
                _peers[i]->_rank = rank;
                _peers[i]->_self = self;
                for (int i = 0, j = _listeners.size() ; i < j ; i++) {
                    _listeners[i]->onPlayerUpdated(name, addr);
                }
            }
            return;
        }
    }
    // Peer is not known. Add to list
    _peers.add(new GamerPeer(name, addr, status, rank, self));
    for (int i = 0, j = _listeners.size() ; i < j ; i++) {
        _listeners[i]->onPlayerConnect(name, addr);
    }
}

void NetGameCenter::disconnectPeer(PeerAddress addr, const std::string &name)
{
    //printf("%s vient de se deconnecter! (parait il)\n", (const char *)name);
    for (int i = 0, j = _peers.size() ; i < j ; i++) {
        GamerPeer *currentPeer = _peers[i];
        if (currentPeer->_address == addr) {
            //printf("Peer trouve au nom %s\n", (const char *)name);
            // Cancels all games from this peer
            for (int i = _pendingGames.size() - 1 ; i >= 0 ; i--) {
                if (_pendingGames[i]->_peer == currentPeer) {
                    for (int u = 0, v = _listeners.size() ; u < v ; u++) {
                        _listeners[u]->onGameInvitationCanceledReceived(_pendingGames[i]->_invitation);
                    }
                    delete _pendingGames[i];
                    _pendingGames.removeAt(i);
                }
            }
            // Delete the disconnected peer
            _peers.remove(currentPeer);
            for (int u = 0, v = _listeners.size() ; u < v ; u++) {
                _listeners[u]->onPlayerDisconnect(currentPeer->_name, currentPeer->_address);
            }
            delete currentPeer;
            return;
        }
    }
    //printf("Pas de peer trouve au nom %s\n", (const char *)name);
}

const std::string & NetGameCenter::getPeerNameAtIndex(int i) const
{
    return _peers[i]->_name;
}

PeerAddress NetGameCenter::getPeerAddressAtIndex(int i) const
{
    return _peers[i]->_address;
}

PeerAddress NetGameCenter::getPeerAddressForPeerName(const std::string & peerName) const
{
    int i;
    for (i = 0 ; i < _peers.size() ; i++) {
        if (_peers[i]->_name == peerName)
            return _peers[i]->_address;
    }
    // hum...
    return _peers[i]->_address;
}

PeerInfo NetGameCenter::getPeerInfoForAddress(PeerAddress &addr) const
{
    PeerInfo info;
    info._status = 0;
    info._rank = -1;
    info._self = false;
    for (int i = 0 ; i < _peers.size() ; i++) {
        if (_peers[i]->_address == addr) {
            info._status = _peers[i]->_status;
            info._rank = _peers[i]->_rank;
            info._self = _peers[i]->_self;
            return info;
        }
    }
    return info;
}

int NetGameCenter::getPeerStatusForAddress(PeerAddress &addr) const
{
    for (int i = 0 ; i < _peers.size() ; i++) {
        if (_peers[i]->_address == addr)
            return _peers[i]->_status;
    }
    return 0;
}

int NetGameCenter::getPeerCount() const
{
    return _peers.size();
}

void NetGameCenter::requestGame(FloboGameInvitation &invitation)
{
    GamerPeer *myPeer = getPeerForAddress(invitation._opponentAddress);
    if (myPeer != NULL) {
        // Checks if there is already an active game with this peer
        for (int i = _pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (_pendingGames[i]->_peer == myPeer)
                return;
        }
        invitation._opponentName = myPeer->_name;
        _pendingGames.add(new PendingGame(myPeer, invitation));
        sendGameRequest(invitation);
    }
}

void NetGameCenter::acceptGameInvitation(FloboGameInvitation &invitation)
{
    GamerPeer *myPeer = getPeerForAddress(invitation._opponentAddress);
    if (myPeer != NULL) {
        sendGameAcceptInvitation(invitation);
    }
}

void NetGameCenter::cancelGameInvitation(FloboGameInvitation &invitation)
{
    GamerPeer *myPeer = getPeerForAddress(invitation._opponentAddress);
    if (myPeer != NULL) {
        for (int i = _pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (_pendingGames[i]->_peer == myPeer) {
                // We won't call gameCanceledAgainst yet to avoid objects destructing themselves
                // but instead set the game in timeout
                _pendingGames[i]->_initiateTime = getTimeMs() - _pendingGameTimeout;
            }
        }
    }
}

void NetGameCenter::grantGameWithMessageBox(FloboGameInvitation &invitation, MessageBox &thembox)
{
    GamerPeer *myPeer = getPeerForAddress(invitation._opponentAddress);
    if (myPeer != NULL) {
        // Remove from pending games
        for (int i = _pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (_pendingGames[i]->_peer == myPeer)
                _pendingGames.removeAt(i);
        }
    }
    setStatus(PEER_PLAYING);
    for (int i = 0, j = _listeners.size() ; i < j ; i++) {
        _listeners[i]->onGameGrantedWithMessagebox(&thembox, invitation);
    }
}

void NetGameCenter::receivedGameInvitation(FloboGameInvitation &invitation)
{
    // Retrieving the peer
    GamerPeer *peer = getPeerForAddress(invitation._opponentAddress);
    if (peer != NULL) {
        // Check if the game doesn't exists
        for (int i = _pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (_pendingGames[i]->_peer == peer)
                return;
        }
        // Create the game
        _pendingGames.add(new PendingGame(peer, invitation));
        // notifies invitation
        for (int i = 0, j = _listeners.size() ; i < j ; i++) {
            _listeners[i]->onGameInvitationReceived(invitation);
        }
    }
}

void NetGameCenter::receivedGameCanceledWithPeer(const std::string & playerName, PeerAddress addr)
{
    // Retrieving the peer
    GamerPeer *peer = getPeerForAddress(addr);
    if (peer != NULL) {
        // Check if the game doesn't exists
        for (int i = _pendingGames.size() - 1 ; i >= 0 ; i--) {
            if (_pendingGames[i]->_peer == peer) {
                // Cancel the game
                for (int u = 0, v = _listeners.size() ; u < v ; u++) {
                    _listeners[u]->onGameInvitationCanceledReceived(_pendingGames[i]->_invitation);
                }
                delete _pendingGames[i];
                _pendingGames.removeAt(i);
            }
        }
    }
}

NetGameCenter::GamerPeer *NetGameCenter::getPeerForAddress(PeerAddress addr)
{
    for (int i = 0 ; i < _peers.size() ; i++) {
        if (_peers[i]->_address == addr) {
            return _peers[i];
        }
    }
    return NULL;
}

