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
      gameGranted(false)
{
    mbox.addListener(this);
    SessionManager &mboxSession = dynamic_cast<SessionManager &>(mbox);
    mboxSession.addSessionListener(this);
    sendAliveMessage();
}

void PuyoLanGameCenter::onMessage(Message &msg)
{
    try {
      switch (msg.getInt("CMD")) {
      case PUYO_UDP_CHAT:
	for (int i = 0, j = listeners.size() ; i < j ; i++) {
	  listeners[i]->onChatMessage(msg.getString("NAME"), msg.getString("MSG"));
	}
	break;
      case PUYO_UDP_ALIVE: {
	  Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
	  connectPeer(dir.getPeerAddress(), msg.getString("NAME"));
        }
	break;
        case PUYO_UDP_DISCONNECT: {
           Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
	   disconnectPeer(dir.getPeerAddress("ADDR"), msg.getString("NAME"));
	}
	break;
      case PUYO_UDP_GAME_REQUEST: {
	  Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
	  receivedInvitationForGameWithPeer(msg.getString("ORGNAME"), dir.getPeerAddress());
        }
	break;
      case PUYO_UDP_GAME_ACCEPT: {
	  Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
	  mbox.bind(dir.getPeerAddress());
	  for (int i = 0, j = listeners.size() ; i < j ; i++) {
	    listeners[i]->gameGrantedWithMessagebox(&mbox);
	  }
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
      grantGameToPeer(grantedAddr);
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

void PuyoLanGameCenter::sendAliveMessage()
{
    Message *msg = mbox.createMessage();
    Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
    dirMsg->setPeerAddress(dirMsg->getBroadcastAddress());

    msg->addInt("CMD", PUYO_UDP_ALIVE);
    msg->addString("NAME", name);
    msg->send();
    delete msg;
}

void PuyoLanGameCenter::requestGameWithPeer(String playerName, PeerAddress addr)
{
  Message *msg = mbox.createMessage();
  Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
  dirMsg->setPeerAddress(addr);

  msg->addBoolProperty("RELIABLE", true);
  msg->addInt("CMD", PUYO_UDP_GAME_REQUEST);
  msg->addString("ORGNAME", name);
  msg->addString("DSTNAME", playerName);
  msg->send();
  delete msg;
}

void PuyoLanGameCenter::acceptInvitationWithPeer(String playerName, PeerAddress addr)
{
  Message *msg = mbox.createMessage();
  Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
  dirMsg->setPeerAddress(addr);

  msg->addBoolProperty("RELIABLE", true);
  msg->addInt("CMD", PUYO_UDP_GAME_ACCEPT);
  msg->addString("ORGNAME", name);
  msg->addString("DSTNAME", playerName);
  msg->send();
  delete msg;
  gameGranted = true;
  grantedAddr = addr;
}

void PuyoLanGameCenter::grantGameToPeer(PeerAddress addr)
{
    mbox.bind(addr);
    for (int i = 0, j = listeners.size() ; i < j ; i++) {
        listeners[i]->gameGrantedWithMessagebox(&mbox);
    }
}

void PuyoLanGameCenter::cancelGameWithPeer(String playerName, PeerAddress addr)
{
  Message *msg = mbox.createMessage();
  Dirigeable *dirMsg = dynamic_cast<Dirigeable *>(msg);
  dirMsg->setPeerAddress(addr);

  msg->addBoolProperty("RELIABLE", true);
  msg->addInt("CMD", PUYO_UDP_GAME_CANCEL);
  msg->addString("ORGNAME", name);
  msg->addString("DSTNAME", playerName);
  msg->send();
  delete msg;
}
