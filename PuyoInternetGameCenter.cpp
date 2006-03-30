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
#include "ios_igpmessage.h"
#include "ios_time.h"

using namespace ios_fc;

PuyoInternetGameCenter::PuyoInternetGameCenter(const String hostName, int portNum, const String name)
  : hostName(hostName), portNum(portNum), mbox(hostName, portNum), p2pmbox(NULL), p2pNatTraversal(NULL), tryNatTraversal(true), name(name), status(PEER_NORMAL),
    timeMsBetweenTwoAliveMessages(3000.), lastAliveMessage(getTimeMs() - timeMsBetweenTwoAliveMessages), gameGrantedStatus(GAMESTATUS_IDLE)
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
    msg->addInt("STATUS", status);
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
    if (tryNatTraversal)
        gameGrantedStatus = GAMESTATUS_STARTTRAVERSAL;
    else
        gameGrantedStatus = GAMESTATUS_GRANTED_IGP;
    grantedAddr = addr;
}

void PuyoInternetGameCenter::grantGameToMBox(MessageBox &thembox)
{
    setStatus(PEER_PLAYING);
    for (int i = 0, j = listeners.size() ; i < j ; i++) {
        listeners[i]->gameGrantedWithMessagebox(&thembox);
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
    switch (gameGrantedStatus) {
        case GAMESTATUS_STARTTRAVERSAL:
            p2pmbox = new UDPMessageBox(hostName, 0, portNum);
	    printf("grantedAddr:%d\n", static_cast<IgpMessage::IgpPeerAddressImpl *>(grantedAddr.getImpl())->getIgpIdent());
            p2pPunchName = "testpunch";
            p2pNatTraversal = new PuyoNatTraversal(*p2pmbox);
            p2pNatTraversal->punch(p2pPunchName);
            gameGrantedStatus = GAMESTATUS_WAITTRAVERSAL;
            break;
        case GAMESTATUS_WAITTRAVERSAL:
            p2pNatTraversal->idle();
            if (p2pNatTraversal->hasFailed()) {
                delete p2pNatTraversal;
                p2pNatTraversal = NULL;
                delete p2pmbox;
                p2pmbox = NULL;
                gameGrantedStatus = GAMESTATUS_GRANTED_IGP;
		printf("NAT traversal failed, falling back to IGP\n");
            }
            else if (p2pNatTraversal->hasSucceeded()) {
                delete p2pNatTraversal;
                p2pNatTraversal = NULL;
                gameGrantedStatus = GAMESTATUS_GRANTED_P2P;
		printf("NAT traversal succeeded, going peer-to-peer\n");
            }
            break;
        case GAMESTATUS_GRANTED_P2P:
            grantGameToMBox(*p2pmbox);
            gameGrantedStatus = GAMESTATUS_IDLE;
            break;
        case GAMESTATUS_GRANTED_IGP:
            mbox.bind(grantedAddr);
            grantGameToMBox(mbox);
            gameGrantedStatus = GAMESTATUS_IDLE;
            break;
        case GAMESTATUS_IDLE:
        default:
            break;
    }
    mbox.idle();
    double time_ms = getTimeMs();
    if ((time_ms - lastAliveMessage) >= timeMsBetweenTwoAliveMessages) {
        sendAliveMessage();
        lastAliveMessage = time_ms;
    }
    PuyoNetGameCenter::idle();
}

void PuyoInternetGameCenter::setStatus(int status)
{
    this->status = status;
    sendAliveMessage();
}

void PuyoInternetGameCenter::punch()
{
    int prevBound = mbox.getBound();
    mbox.bind(1);
    Message *msg = mbox.createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", PUYO_IGP_NAT_TRAVERSAL);
    msg->send();
    delete msg;
    mbox.bind(prevBound);
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
                connectPeer(dir.getPeerAddress("ADDR"), msg.getString("NAME"), msg.getInt("STATUS"));
            }
                break;
            case PUYO_IGP_DISCONNECT:
            {
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                disconnectPeer(dir.getPeerAddress("ADDR"), msg.getString("NAME"));
            }
                break;
            case PUYO_IGP_STATUSCHANGE:
            {
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                connectPeer(dir.getPeerAddress("ADDR"), msg.getString("NAME"), msg.getInt("STATUS"));
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
    	        setStatus(PEER_PLAYING);
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                grantedAddr = dir.getPeerAddress();
                if (tryNatTraversal)
                    gameGrantedStatus = GAMESTATUS_STARTTRAVERSAL;
                else
                    gameGrantedStatus = GAMESTATUS_GRANTED_IGP; 
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

