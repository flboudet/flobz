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

#include "PuyoInternetGameCenter.h"
#include "PuyoIgpDefs.h"
#include "ios_time.h"

using namespace ios_fc;

PuyoInternetGameCenter::PuyoInternetGameCenter(const String hostName, int portNum, const String name)
  : mbox(hostName, portNum), name(name),
    timeMsBetweenTwoAliveMessages(3000.), lastAliveMessage(getTimeMs() - timeMsBetweenTwoAliveMessages), gameGranted(false)
{
    mbox.addListener(this);
    sendAliveMessage();
}

void PuyoInternetGameCenter::sendAliveMessage()
{
    int prevBound = mbox.getBound();
    mbox.bind(1);
    Message *msg = mbox.createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", PUYO_IGP_ALIVE);
    msg->addString("NAME", name);
    msg->send();
    delete msg;
    mbox.bind(prevBound);
}

void PuyoInternetGameCenter::sendMessage(const String msgText)
{
    //printf("Envoi du msg:%s\n", (const char *)msgText);
    int prevBound = mbox.getBound();
    mbox.bind(1);
    Message *msg = mbox.createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", PUYO_IGP_CHAT);
    msg->addString("NAME", name);
    msg->addString("MSG", msgText);
    msg->send();
    delete msg;
    mbox.bind(prevBound);
}

void PuyoInternetGameCenter::requestGameWithPeer(String playerName, PeerAddress addr)
{
    Message *msg = mbox.createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", PUYO_IGP_GAME_REQUEST);
    msg->addString("ORGNAME", name);
    msg->addString("DSTNAME", playerName);
    Dirigeable *dirNew = dynamic_cast<Dirigeable *>(msg);
    dirNew->setPeerAddress(addr);
    msg->send();
    delete msg;
}

void PuyoInternetGameCenter::acceptInvitationWithPeer(String playerName, PeerAddress addr)
{
    Message *msg = mbox.createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", PUYO_IGP_GAME_ACCEPT);
    msg->addString("ORGNAME", name);
    msg->addString("DSTNAME", playerName);
    Dirigeable *dirNew = dynamic_cast<Dirigeable *>(msg);
    dirNew->setPeerAddress(addr);
    msg->send();
    delete msg;
    gameGranted = true;
    grantedAddr = addr;
}

void PuyoInternetGameCenter::grantGameToPeer(PeerAddress addr)
{
    mbox.bind(addr);
    for (int i = 0, j = listeners.size() ; i < j ; i++) {
        listeners[i]->gameGrantedWithMessagebox(&mbox);
    }
}

void PuyoInternetGameCenter::cancelGameWithPeer(String playerName, PeerAddress addr)
{
    Message *msg = mbox.createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", PUYO_IGP_GAME_CANCEL);
    msg->addString("ORGNAME", name);
    msg->addString("DSTNAME", playerName);
    Dirigeable *dirNew = dynamic_cast<Dirigeable *>(msg);
    dirNew->setPeerAddress(addr);
    msg->send();
    delete msg;
}

void PuyoInternetGameCenter::idle()
{
    static int idleCount = 0;
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

void PuyoInternetGameCenter::onMessage(Message &msg)
{
    //printf("Cool, un msg!\n");
    try {
        if (!msg.hasInt("CMD"))
            return;
        switch (msg.getInt("CMD")) {
            case PUYO_IGP_CHAT:
                for (int i = 0, j = listeners.size() ; i < j ; i++) {
                    listeners[i]->onChatMessage(msg.getString("NAME"), msg.getString("MSG"));
                }
                break;
            case PUYO_IGP_CONNECT:
            {
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                connectPeer(dir.getPeerAddress("ADDR"), msg.getString("NAME"));
            }
                break;
            case PUYO_IGP_DISCONNECT:
            {
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                disconnectPeer(dir.getPeerAddress("ADDR"), msg.getString("NAME"));
            }
                break;
            case PUYO_IGP_GAME_REQUEST:
            {
                //printf("Une partie contre %s?\n", (const char *)msg.getString("ORGNAME"));
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                receivedInvitationForGameWithPeer(msg.getString("ORGNAME"), dir.getPeerAddress());
            }
                break;
            case PUYO_IGP_GAME_ACCEPT:
            {
                //printf("%s accepte la partie!\n", (const char *)msg.getString("ORGNAME"));
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                mbox.bind(dir.getPeerAddress());
                for (int i = 0, j = listeners.size() ; i < j ; i++) {
                    listeners[i]->gameGrantedWithMessagebox(&mbox);
                }
            }
                break;
            case PUYO_IGP_GAME_CANCEL:
            {
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                receivedGameCanceledWithPeer(msg.getString("ORGNAME"), dir.getPeerAddress());
            }
                break;
            default:
                break;
        }
    }
    catch (Exception e) {
        printf("Message invalide 3!\n");
    }
}

