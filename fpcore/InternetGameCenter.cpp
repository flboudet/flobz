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

#include <iostream>
#include "GTLog.h"
#include "InternetGameCenter.h"
#include "FPIGPDefs.h"
#include "ios_igpmessage.h"
#include "ios_time.h"

using namespace std;
using namespace ios_fc;

const int InternetGameCenter::fpipVersion = 0x00000001;

InternetGameCenter::InternetGameCenter(const std::string &hostName, int portNum, const std::string &name, const std::string &password)
  : _hostName(hostName), _portNum(portNum), _tryNatTraversal(true),
    _name(name), _password(password), _status(PEER_NORMAL),
    _timeMsBetweenTwoAliveMessages(3000.), _lastAliveMessage(getTimeMs() - _timeMsBetweenTwoAliveMessages), _gameGrantedStatus(GAMESTATUS_IDLE),
    _isAccepted(false), _isDenied(false), _denyString(""), _denyStringMore("")
{
    _udpSocket.reset(new DatagramSocket());
    _udpSocket->connect(SocketAddress(_hostName.c_str()), _portNum);
    _udpmbox.reset(new FPServerMessageBox(_udpSocket.get()));
    _igpmbox.reset(new FPServerIGPMessageBox(_udpmbox.get()));
    _igpmbox->addListener(this);
    sendAliveMessage();
}

void InternetGameCenter::sendAliveMessage()
{
    int prevBound = _igpmbox->getBound();
    _igpmbox->bind(1);
    Message *msg = _igpmbox->createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("V", fpipVersion);
    msg->addInt("CMD", FLOBO_IGP_ALIVE);
    msg->addString("NAME", _name.c_str());
    msg->addString("PASSWD", _password.c_str());
    msg->addInt("STATUS", _status);
    msg->send();
    delete msg;
    _igpmbox->bind(prevBound);
}

void InternetGameCenter::sendMessage(const std::string &msgText)
{
    //printf("Envoi du msg:%s\n", (const char *)msgText);
    int prevBound = _igpmbox->getBound();
    _igpmbox->bind(1);
    Message *msg = _igpmbox->createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("V", fpipVersion);
    msg->addInt("CMD", FLOBO_IGP_CHAT);
    msg->addString("NAME", _name.c_str());
    msg->addString("MSG", msgText.c_str());
    msg->send();
    delete msg;
    _igpmbox->bind(prevBound);
}

void InternetGameCenter::sendGameRequest(FloboGameInvitation &invitation)
{
    _opponentName = invitation._opponentName;
    invitation._initiatorAddress = _igpmbox->getSelfAddress();
    Message *msg = _igpmbox->createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", FLOBO_IGP_GAME_REQUEST);
    msg->addString("ORGNAME", _name.c_str());
    msg->addString("DSTNAME", invitation._opponentName.c_str());
    msg->addInt("RNDSEED", invitation._gameRandomSeed);
    msg->addInt("SPEED", invitation._gameSpeed);
    msg->addInt("GSETS", invitation._gameNbSets);
    Dirigeable *dirNew = dynamic_cast<Dirigeable *>(msg);
    dirNew->setPeerAddress(invitation._opponentAddress);
    msg->send();
    delete msg;
    _grantedInvitation = invitation;
}

void InternetGameCenter::sendGameAcceptInvitation(FloboGameInvitation &invitation)
{
    _opponentName = invitation._opponentName;
    Message *msg = _igpmbox->createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", FLOBO_IGP_GAME_ACCEPT);
    msg->addString("ORGNAME", _name.c_str());
    msg->addString("DSTNAME", invitation._opponentName.c_str());
    Dirigeable *dirNew = dynamic_cast<Dirigeable *>(msg);
    dirNew->setPeerAddress(invitation._opponentAddress);
    msg->send();
    delete msg;
    if (_tryNatTraversal)
        _gameGrantedStatus = GAMESTATUS_STARTTRAVERSAL;
    else
        _gameGrantedStatus = GAMESTATUS_GRANTED_IGP;
    _grantedInvitation = invitation;
}

void InternetGameCenter::sendGameCancelInvitation(FloboGameInvitation &invitation)
{
    Message *msg = _igpmbox->createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", FLOBO_IGP_GAME_CANCEL);
    msg->addString("ORGNAME", _name.c_str());
    msg->addString("DSTNAME", invitation._opponentName.c_str());
    Dirigeable *dirNew = dynamic_cast<Dirigeable *>(msg);
    dirNew->setPeerAddress(invitation._opponentAddress);
    msg->send();
    delete msg;
}

void InternetGameCenter::idle()
{
    switch (_gameGrantedStatus) {
        case GAMESTATUS_STARTTRAVERSAL:
            // The game has been accepted, negociation is pending
            for (int i = 0, j = _listeners.size() ; i < j ; i++) {
                _listeners[i]->onGameAcceptedNegociationPending(_grantedInvitation);
            }
            _p2pSocket.reset(new DatagramSocket());
            _p2pSocket->connect(SocketAddress(_hostName.c_str()), _portNum);
            _p2pmbox.reset(new FPInternetP2PMessageBox(_p2pSocket.get()));
            //printf("grantedAddr:%d\n", static_cast<IgpMessage::IgpPeerAddressImpl *>(_grantedInvitation._opponentAddress.getImpl())->getIgpIdent());

            int initiatorIgpIdent, guestIgpIdent;
            if (_grantedInvitation._initiatorAddress == _grantedInvitation._opponentAddress) {
                // The opponent invited me
                initiatorIgpIdent = static_cast<IgpPeerAddressImpl *>(_grantedInvitation._initiatorAddress.getImpl())->getIgpIdent();
                guestIgpIdent = static_cast<IgpPeerAddressImpl *>(_igpmbox->getSelfAddress().getImpl())->getIgpIdent();
            }
            else {
                // I invited the opponent
                initiatorIgpIdent = static_cast<IgpPeerAddressImpl *>(_grantedInvitation._initiatorAddress.getImpl())->getIgpIdent();
                guestIgpIdent = static_cast<IgpPeerAddressImpl *>(_grantedInvitation._opponentAddress.getImpl())->getIgpIdent();
            }
            _p2pPunchName = std::string("punch:") + std::to_string(initiatorIgpIdent) + "vs" + std::to_string(guestIgpIdent) + ":" + std::to_string(_grantedInvitation._gameRandomSeed);
            _p2pNatTraversal.reset(new NatTraversal(*_p2pmbox));
            _p2pNatTraversal->punch(_p2pPunchName.c_str());
            _gameGrantedStatus = GAMESTATUS_WAITTRAVERSAL;
            break;
        case GAMESTATUS_WAITTRAVERSAL:
            _p2pNatTraversal->idle();
            if (_p2pNatTraversal->hasFailed()) {
                _p2pNatTraversal.reset();
                _p2pmbox.reset();
                _gameGrantedStatus = GAMESTATUS_GRANTED_IGP;
                GTLogTrace("NAT traversal failed, falling back to IGP");
            }
            else if (_p2pNatTraversal->hasSucceeded()) {
                _p2pNatTraversal.reset();
                _gameGrantedStatus = GAMESTATUS_GRANTED_P2P;
                GTLogTrace("NAT traversal succeeded, going peer-to-peer");
            }
            break;
        case GAMESTATUS_GRANTED_P2P:
            grantGameWithMessageBox(_grantedInvitation, *_p2pmbox);
            _gameGrantedStatus = GAMESTATUS_IDLE;
            break;
        case GAMESTATUS_GRANTED_IGP:
            _igpmbox->bind(_grantedInvitation._opponentAddress);
            grantGameWithMessageBox(_grantedInvitation, *_p2pmbox);
            _gameGrantedStatus = GAMESTATUS_IDLE;
            break;
        case GAMESTATUS_IDLE:
        default:
            break;
    }
    _igpmbox->idle();
    double time_ms = getTimeMs();
    if ((time_ms - _lastAliveMessage) >= _timeMsBetweenTwoAliveMessages) {
        sendAliveMessage();
        _lastAliveMessage = time_ms;
    }
    NetGameCenter::idle();
}

void InternetGameCenter::setStatus(int status)
{
    this->_status = status;
    sendAliveMessage();
}

const std::string & InternetGameCenter::getSelfName()
{
    return _name;
}

const std::string &InternetGameCenter::getOpponentName()
{
    return _opponentName;
}

bool InternetGameCenter::isConnected() const
{
    return (_igpmbox->isConnected()) && (_isAccepted);
}

bool InternetGameCenter::isDenied() const
{
    return (_igpmbox->isConnected()) && (_isDenied);
}

void InternetGameCenter::punch()
{
    int prevBound = _igpmbox->getBound();
    _igpmbox->bind(1);
    Message *msg = _igpmbox->createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", FLOBO_IGP_NAT_TRAVERSAL);
    msg->send();
    delete msg;
    _igpmbox->bind(prevBound);
}

void InternetGameCenter::onMessage(Message &msg)
{
    //printf("Cool, un msg!\n");
    try {
        if (!msg.hasInt("CMD"))
            return;
        switch (msg.getInt("CMD")) {
            case FLOBO_IGP_ACCEPT:
                _isAccepted = true;
                break;
            case FLOBO_IGP_DENY:
                _isDenied = true;
                _denyString = msg.getString("MSG").c_str();
                if (msg.hasString("MSG_MORE"))
                    _denyStringMore = msg.getString("MSG_MORE").c_str();
                else
                    _denyStringMore = "";
                break;
            case FLOBO_IGP_CHAT:
                for (int i = 0, j = _listeners.size() ; i < j ; i++) {
                    _listeners[i]->onChatMessage(msg.getString("NAME").c_str(), msg.getString("MSG").c_str());
                }
                break;
            case FLOBO_IGP_CONNECT:
            {
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                int rank = 0;
                if (msg.hasInt("RANK"))
                    rank = msg.getInt("RANK");
                PeerAddress peerAddress = dir.getPeerAddress("ADDR");
                PeerAddress selfAddress = _igpmbox->getSelfAddress();
                connectPeer(peerAddress, msg.getString("NAME").c_str(), msg.getInt("STATUS"), rank, peerAddress == selfAddress);
            }
                break;
            case FLOBO_IGP_DISCONNECT:
            {
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                disconnectPeer(dir.getPeerAddress("ADDR"), msg.getString("NAME").c_str());
            }
                break;
            case FLOBO_IGP_STATUSCHANGE:
            {
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                int rank = 0;
                if (msg.hasInt("RANK"))
                    rank = msg.getInt("RANK");
                connectPeer(dir.getPeerAddress("ADDR"), msg.getString("NAME").c_str(), msg.getInt("STATUS"), rank);
            }
                break;
            case FLOBO_IGP_GAME_REQUEST:
            {
                //printf("Une partie contre %s?\n", (const char *)msg.getString("ORGNAME"));
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                FloboGameInvitation invitation;
                invitation._initiatorAddress = dir.getPeerAddress();
                invitation._opponentAddress = dir.getPeerAddress();
                invitation._opponentName = msg.getString("ORGNAME").c_str();
                if (msg.hasInt("RNDSEED"))
                    invitation._gameRandomSeed = msg.getInt("RNDSEED");
                else
                    invitation._gameRandomSeed = 0; // When there is no seed, fall back to 0 (better than crashing)
                if (msg.hasInt("SPEED"))
                    invitation._gameSpeed = msg.getInt("SPEED");
                else
                    invitation._gameSpeed = 1; // When there is no seed, fall back to 1 (better than crashing)
                if (msg.hasInt("GSETS"))
                    invitation._gameNbSets = msg.getInt("GSETS");
                else
                    invitation._gameNbSets = 0;
                receivedGameInvitation(invitation);
            }
                break;
            case FLOBO_IGP_GAME_ACCEPT:
            {
                //printf("%s accepte la partie!\n", (const char *)msg.getString("ORGNAME"));
    	        setStatus(PEER_PLAYING);
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                if (!(_grantedInvitation._opponentAddress == dir.getPeerAddress()))
                    break;
                if (_tryNatTraversal)
                    _gameGrantedStatus = GAMESTATUS_STARTTRAVERSAL;
                else
                    _gameGrantedStatus = GAMESTATUS_GRANTED_IGP;
            }
                break;
            case FLOBO_IGP_GAME_CANCEL:
            {
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                receivedGameCanceledWithPeer(msg.getString("ORGNAME").c_str(), dir.getPeerAddress());
            }
                break;
            default:
                break;
        }
    }
    catch (const std::exception &e) {
        printf("Message invalide 3!\n");
    }
}

