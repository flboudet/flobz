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

#include "LanGameCenter.h"
#include "ios_time.h"

#define MULTICASTGROUP "224.0.0.247"

using namespace ios_fc;

enum {
    FLOBO_UDP_ALIVE,
    FLOBO_UDP_DISCONNECT,
    FLOBO_UDP_CHAT,
    FLOBO_UDP_GAME_REQUEST,
    FLOBO_UDP_GAME_ACCEPT,
    FLOBO_UDP_GAME_CANCEL
};

LanGameCenter::LanGameCenter(int portNum, const std::string &name)
    : _socket(portNum), _mbox(&_socket), _name(name),
      _timeMsBetweenTwoAliveMessages(3000.), _lastAliveMessage(getTimeMs() - _timeMsBetweenTwoAliveMessages),
      _timeMsBetweenTwoNetworkInterfacesDetection(10000.), _lastNetworkInterfacesDetection(getTimeMs()),
      _gameGranted(false), _status(PEER_NORMAL), _multicastAddress(MULTICASTGROUP), _loopbackAddress("127.0.0.1"),
      _networkInterfaces(_requester.getInterfaces()), _mcastPeerAddress(_multicastAddress, portNum)
{
    _uuid = (int)((int64_t)getTimeMs() % 0xFFFFFFFF);
    _socket.joinGroup(_multicastAddress);
    _mbox.addListener(this);
    SessionManager &mboxSession = dynamic_cast<SessionManager &>(_mbox);
    mboxSession.addSessionListener(this);
    addListener(this);
    sendAliveMessage();
}

LanGameCenter::~LanGameCenter()
{
    sendDisconnectMessage();
}

void LanGameCenter::onMessage(Message &msg)
{
    try {
      if (!msg.hasInt("CMD"))
            return;
      switch (msg.getInt("CMD")) {
      case FLOBO_UDP_CHAT:
	for (int i = 0, j = _listeners.size() ; i < j ; i++) {
	  _listeners[i]->onChatMessage(msg.getString("NAME").c_str(), msg.getString("MSG").c_str());
	}
	break;
      case FLOBO_UDP_ALIVE: {
          Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
          int uuid = msg.getInt("UUID");
          int status = msg.getInt("STATUS");
          bool self = false;
          if (uuid == _uuid) {
              self = true;
          }
          NetGameCenter::connectPeer(dir.getPeerAddress(), msg.getString("NAME").c_str(), status, -1, self);
      }
	break;
      case FLOBO_UDP_DISCONNECT: {
          Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
          //printf("Message de deconnexion recu de %s...\n", (const char *)(msg.getString("NAME")));
          NetGameCenter::disconnectPeer(dir.getPeerAddress(), msg.getString("NAME").c_str());
      }
	break;
        case FLOBO_UDP_GAME_REQUEST: {
            Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
            FloboGameInvitation invitation;
            invitation._opponentAddress = dir.getPeerAddress();
            invitation._opponentName = msg.getString("ORGNAME");
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
      case FLOBO_UDP_GAME_ACCEPT: {
          Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
          if (_grantedInvitation._opponentAddress == dir.getPeerAddress())
            grantGame(_grantedInvitation);
      }
	break;
      case FLOBO_UDP_GAME_CANCEL:  {
          Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
	  receivedGameCanceledWithPeer(msg.getString("ORGNAME").c_str(), dir.getPeerAddress());
        }
	break;
      default:
	break;
      }
    }
    catch (const std::exception &e) {
        printf("Une putain d'exception est survenue: %s\n", e.what());
    }
}

void LanGameCenter::onPeerDisconnect(const PeerAddress &address)
{
  disconnectPeer(address, "Unknown");
}

void LanGameCenter::sendMessage(const std::string &msgText)
{
  for (int i = 0, j = getPeerCount() ; i < j ; i++) {
    Message *msg = _mbox.createMessage();
    Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
    dirMsg->setPeerAddress(getPeerAddressAtIndex(i));

    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", FLOBO_UDP_CHAT);
    msg->addString("NAME", _name.c_str());
    msg->addString("MSG", msgText.c_str());
    msg->send();
    delete msg;
  }
}

void LanGameCenter::idle()
{
    if (_gameGranted) {
      grantGame(_grantedInvitation);
      _gameGranted = false;
      return;
    }
    _mbox.idle();
    double time_ms = getTimeMs();
    if ((time_ms - _lastAliveMessage) >= _timeMsBetweenTwoAliveMessages) {
        sendAliveMessage();
        _lastAliveMessage = time_ms;
    }
    if ((time_ms - _lastNetworkInterfacesDetection) >= _timeMsBetweenTwoNetworkInterfacesDetection) {
        _networkInterfaces = _requester.getInterfaces();
        _lastNetworkInterfacesDetection = time_ms;
    }
    NetGameCenter::idle();
}

void LanGameCenter::setStatus(int status)
{
    this->_status = status;
    sendAliveMessage();
}

const std::string &LanGameCenter::getSelfName()
{
    return _name;
}

const std::string &LanGameCenter::getOpponentName()
{
    return _opponentName;
}

void LanGameCenter::sendAliveMessage()
{
    try {
        for (unsigned int i = 0 ; i < _networkInterfaces.size() ; i++) {
            NetworkInterface &ifs = _networkInterfaces[i];
            if (ifs.getAddress() == _loopbackAddress)
                continue;
            _socket.setMulticastInterface(ifs.getAddress());
            Message *msg = _mbox.createMessage();
            Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
            dirMsg->setPeerAddress(_mcastPeerAddress);

            msg->addInt("CMD", FLOBO_UDP_ALIVE);
            msg->addString("NAME", _name.c_str());
            msg->addInt("STATUS", _status);
            msg->addInt("UUID", _uuid);
            msg->send();
            delete msg;
        }
    }
    catch (const std::exception &ex) {
        // If we have an exception there, it is probably because we are attempting to use
        // a network interface which has just disappeared. Redetect the interfaces.
        _networkInterfaces = _requester.getInterfaces();
    }
}

void LanGameCenter::sendDisconnectMessage()
{
    for (size_t i = 0 ; i < _networkInterfaces.size() ; i++) {
        NetworkInterface &ifs = _networkInterfaces[i];
        if (ifs.getAddress() == _loopbackAddress)
            continue;
        _socket.setMulticastInterface(ifs.getAddress());
        Message *msg = _mbox.createMessage();
        Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
        dirMsg->setPeerAddress(_mcastPeerAddress);

        msg->addInt("CMD", FLOBO_UDP_DISCONNECT);
        msg->addString("NAME", _name.c_str());
        msg->send();
        //printf("Message de deconnexion envoye...\n");
        delete msg;
    }
}

void LanGameCenter::sendGameRequest(FloboGameInvitation &invitation)
{
  _opponentName = invitation._opponentName;
  Message *msg = _mbox.createMessage();
  Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
  dirMsg->setPeerAddress(invitation._opponentAddress);

  msg->addBoolProperty("RELIABLE", true);
  msg->addInt("CMD", FLOBO_UDP_GAME_REQUEST);
  msg->addString("ORGNAME", _name.c_str());
  msg->addString("DSTNAME", invitation._opponentName.c_str());
  msg->addInt("RNDSEED", invitation._gameRandomSeed);
  msg->addInt("SPEED", invitation._gameSpeed);
  msg->addInt("GSETS", invitation._gameNbSets);
  msg->send();
  delete msg;
  _grantedInvitation = invitation;
}

void LanGameCenter::sendGameAcceptInvitation(FloboGameInvitation &invitation)
{
  _opponentName = invitation._opponentName;
  Message *msg = _mbox.createMessage();
  Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
  dirMsg->setPeerAddress(invitation._opponentAddress);

  msg->addBoolProperty("RELIABLE", true);
  msg->addInt("CMD", FLOBO_UDP_GAME_ACCEPT);
  msg->addString("ORGNAME", _name.c_str());
  msg->addString("DSTNAME", invitation._opponentName.c_str());
  msg->send();
  delete msg;
  _gameGranted = true;
  _grantedInvitation = invitation;
}

void LanGameCenter::grantGame(FloboGameInvitation &invitation)
{
    _mbox.bind(invitation._opponentAddress);
    grantGameWithMessageBox(invitation, _mbox);
}

void LanGameCenter::sendGameCancelInvitation(FloboGameInvitation &invitation)
{
  Message *msg = _mbox.createMessage();
  Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
  dirMsg->setPeerAddress(invitation._opponentAddress);

  msg->addBoolProperty("RELIABLE", true);
  msg->addInt("CMD", FLOBO_UDP_GAME_CANCEL);
  msg->addString("ORGNAME", _name.c_str());
  msg->addString("DSTNAME", invitation._opponentName.c_str());
  msg->send();
  delete msg;
}

void LanGameCenter::onPlayerConnect(const std::string &playerName, PeerAddress playerAddress)
{
    // When a new player connects, send an alive message
    sendAliveMessage();
}
