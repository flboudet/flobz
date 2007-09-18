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

#include "PuyoLanGameCenter.h"
#include "ios_time.h"

#define MULTICASTGROUP "224.0.0.247"

using namespace ios_fc;

enum {
    PUYO_UDP_ALIVE,
    PUYO_UDP_DISCONNECT,
    PUYO_UDP_CHAT,
    PUYO_UDP_GAME_REQUEST,
    PUYO_UDP_GAME_ACCEPT,
    PUYO_UDP_GAME_CANCEL
};

PuyoLanGameCenter::PuyoLanGameCenter(int portNum, const String name)
    : socket(portNum), mbox(&socket), name(name),
      timeMsBetweenTwoAliveMessages(3000.), lastAliveMessage(getTimeMs() - timeMsBetweenTwoAliveMessages),
      gameGranted(false), status(PEER_NORMAL), multicastAddress(MULTICASTGROUP), loopbackAddress("127.0.0.1"),
      networkInterfaces(requester.getInterfaces()), mcastPeerAddress(multicastAddress, portNum)
{
    socket.joinGroup(multicastAddress);
    mbox.addListener(this);
    SessionManager &mboxSession = dynamic_cast<SessionManager &>(mbox);
    mboxSession.addSessionListener(this);
    addListener(this);
    sendAliveMessage();
}

PuyoLanGameCenter::~PuyoLanGameCenter()
{
    sendDisconnectMessage();
}

void PuyoLanGameCenter::onMessage(Message &msg)
{
    try {
      if (!msg.hasInt("CMD"))
            return;
      switch (msg.getInt("CMD")) {
      case PUYO_UDP_CHAT:
	for (int i = 0, j = listeners.size() ; i < j ; i++) {
	  listeners[i]->onChatMessage(msg.getString("NAME"), msg.getString("MSG"));
	}
	break;
      case PUYO_UDP_ALIVE: {
	  Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
	  PuyoNetGameCenter::connectPeer(dir.getPeerAddress(), msg.getString("NAME"), msg.getInt("STATUS"));
        }
	break;
      case PUYO_UDP_DISCONNECT: {
          Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
          //printf("Message de deconnexion recu de %s...\n", (const char *)(msg.getString("NAME")));
          PuyoNetGameCenter::disconnectPeer(dir.getPeerAddress(), msg.getString("NAME"));
      }
	break;
        case PUYO_UDP_GAME_REQUEST: {
            Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
            PuyoGameInvitation invitation;
            invitation.opponentAddress = dir.getPeerAddress();
            invitation.opponentName = msg.getString("ORGNAME");
            if (msg.hasInt("RNDSEED"))
                invitation.gameRandomSeed = msg.getInt("RNDSEED");
            else
                invitation.gameRandomSeed = 0; // When there is no seed, fall back to 0 (better than crashing)
            receivedGameInvitation(invitation);
        }
	break;
      case PUYO_UDP_GAME_ACCEPT: {
          Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
          if (grantedInvitation.opponentAddress == dir.getPeerAddress());
            grantGame(grantedInvitation);
      }
	break;
      case PUYO_UDP_GAME_CANCEL:  {
          Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
	  receivedGameCanceledWithPeer(msg.getString("ORGNAME"), dir.getPeerAddress());
        }
	break;
      default:
	break;
      }
    }
    catch (Exception e) {
        printf("Une putain d'exception est survenue: %s\n", e.what());
    }
}

void PuyoLanGameCenter::onPeerDisconnect(const PeerAddress &address)
{
  disconnectPeer(address, String("Unknown"));
}

void PuyoLanGameCenter::sendMessage(const String msgText)
{
  for (int i = 0, j = getPeerCount() ; i < j ; i++) {
    Message *msg = mbox.createMessage();
    Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
    dirMsg->setPeerAddress(getPeerAddressAtIndex(i));

    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", PUYO_UDP_CHAT);
    msg->addString("NAME", name);
    msg->addString("MSG", msgText);
    msg->send();
    delete msg;
  }
}

void PuyoLanGameCenter::idle()
{
    if (gameGranted) {
      grantGame(grantedInvitation);
      gameGranted = false;
      return;
    }
    mbox.idle();
    double time_ms = getTimeMs();
    if ((time_ms - lastAliveMessage) >= timeMsBetweenTwoAliveMessages) {
        sendAliveMessage();
        lastAliveMessage = time_ms;
    }
    PuyoNetGameCenter::idle();
}

void PuyoLanGameCenter::setStatus(int status)
{
    this->status = status;
    sendAliveMessage();
}

String PuyoLanGameCenter::getSelfName()
{
    return name;
}

String PuyoLanGameCenter::getOpponentName()
{
    return opponentName;
}

void PuyoLanGameCenter::sendAliveMessage()
{
    for (int i = 0 ; i < networkInterfaces.size() ; i++) {
        NetworkInterface &ifs = networkInterfaces[i];
        if (ifs.getAddress() == loopbackAddress)
            continue;
        socket.setMulticastInterface(ifs.getAddress());
        Message *msg = mbox.createMessage();
        Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
        dirMsg->setPeerAddress(mcastPeerAddress);
        
        msg->addInt("CMD", PUYO_UDP_ALIVE);
        msg->addString("NAME", name);
        msg->addInt("STATUS", status);
        msg->send();
        delete msg;
    }
}

void PuyoLanGameCenter::sendDisconnectMessage()
{
    for (int i = 0 ; i < networkInterfaces.size() ; i++) {
        NetworkInterface &ifs = networkInterfaces[i];
        if (ifs.getAddress() == loopbackAddress)
            continue;
        socket.setMulticastInterface(ifs.getAddress());
        Message *msg = mbox.createMessage();
        Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
        dirMsg->setPeerAddress(mcastPeerAddress);
        
        msg->addInt("CMD", PUYO_UDP_DISCONNECT);
        msg->addString("NAME", name);
        msg->send();
        //printf("Message de deconnexion envoye...\n");
        delete msg;
    }
}

void PuyoLanGameCenter::sendGameRequest(PuyoGameInvitation &invitation)
{
  opponentName = invitation.opponentName;
  Message *msg = mbox.createMessage();
  Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
  dirMsg->setPeerAddress(invitation.opponentAddress);

  msg->addBoolProperty("RELIABLE", true);
  msg->addInt("CMD", PUYO_UDP_GAME_REQUEST);
  msg->addString("ORGNAME", name);
  msg->addString("DSTNAME", invitation.opponentName);
  msg->addInt("RNDSEED", invitation.gameRandomSeed);
  msg->send();
  delete msg;
  grantedInvitation = invitation;
}

void PuyoLanGameCenter::sendGameAcceptInvitation(PuyoGameInvitation &invitation)
{
  opponentName = invitation.opponentName;
  Message *msg = mbox.createMessage();
  Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
  dirMsg->setPeerAddress(invitation.opponentAddress);

  msg->addBoolProperty("RELIABLE", true);
  msg->addInt("CMD", PUYO_UDP_GAME_ACCEPT);
  msg->addString("ORGNAME", name);
  msg->addString("DSTNAME", invitation.opponentName);
  msg->send();
  delete msg;
  gameGranted = true;
  grantedInvitation = invitation;
}

void PuyoLanGameCenter::grantGame(PuyoGameInvitation &invitation)
{
    setStatus(PEER_PLAYING);
    mbox.bind(invitation.opponentAddress);
    for (int i = 0, j = listeners.size() ; i < j ; i++) {
        listeners[i]->onGameGrantedWithMessagebox(&mbox, invitation);
    }
}

void PuyoLanGameCenter::sendGameCancelInvitation(PuyoGameInvitation &invitation)
{
  Message *msg = mbox.createMessage();
  Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
  dirMsg->setPeerAddress(invitation.opponentAddress);

  msg->addBoolProperty("RELIABLE", true);
  msg->addInt("CMD", PUYO_UDP_GAME_CANCEL);
  msg->addString("ORGNAME", name);
  msg->addString("DSTNAME", invitation.opponentName);
  msg->send();
  delete msg;
}

void PuyoLanGameCenter::onPlayerConnect(String playerName, PeerAddress playerAddress)
{
    // When a new player connects, send an alive message
    sendAliveMessage();
}
