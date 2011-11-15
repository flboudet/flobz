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
#include "PuyoIgpDefs.h"
#include "ios_igpmessage.h"
#include "ios_time.h"

using namespace std;
using namespace ios_fc;

const int InternetGameCenter::fpipVersion = 0x00000001;

InternetGameCenter::InternetGameCenter(const String hostName, int portNum, const String name, const String password)
  : hostName(hostName), portNum(portNum), tryNatTraversal(true),
    name(name), password(password), status(PEER_NORMAL),
    timeMsBetweenTwoAliveMessages(3000.), lastAliveMessage(getTimeMs() - timeMsBetweenTwoAliveMessages), gameGrantedStatus(GAMESTATUS_IDLE),
    m_isAccepted(false), m_isDenied(false), m_denyString(""), m_denyStringMore("")
{
    m_udpSocket.reset(new DatagramSocket());
    m_udpSocket->connect(SocketAddress(hostName), portNum);
    m_udpmbox.reset(new FPServerMessageBox(m_udpSocket.get()));
    m_igpmbox.reset(new FPServerIGPMessageBox(m_udpmbox.get()));
    m_igpmbox->addListener(this);
    sendAliveMessage();
}

void InternetGameCenter::sendAliveMessage()
{
    int prevBound = m_igpmbox->getBound();
    m_igpmbox->bind(1);
    Message *msg = m_igpmbox->createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("V", fpipVersion);
    msg->addInt("CMD", FLOBO_IGP_ALIVE);
    msg->addString("NAME", name);
    msg->addString("PASSWD", password);
    msg->addInt("STATUS", status);
    msg->send();
    delete msg;
    m_igpmbox->bind(prevBound);
}

void InternetGameCenter::sendMessage(const String msgText)
{
    //printf("Envoi du msg:%s\n", (const char *)msgText);
    int prevBound = m_igpmbox->getBound();
    m_igpmbox->bind(1);
    Message *msg = m_igpmbox->createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("V", fpipVersion);
    msg->addInt("CMD", FLOBO_IGP_CHAT);
    msg->addString("NAME", name);
    msg->addString("MSG", msgText);
    msg->send();
    delete msg;
    m_igpmbox->bind(prevBound);
}

void InternetGameCenter::sendGameRequest(FloboGameInvitation &invitation)
{
    opponentName = invitation.opponentName;
    invitation.initiatorAddress = m_igpmbox->getSelfAddress();
    Message *msg = m_igpmbox->createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", FLOBO_IGP_GAME_REQUEST);
    msg->addString("ORGNAME", name);
    msg->addString("DSTNAME", invitation.opponentName);
    msg->addInt("RNDSEED", invitation.gameRandomSeed);
    msg->addInt("SPEED", invitation.gameSpeed);
    Dirigeable *dirNew = dynamic_cast<Dirigeable *>(msg);
    dirNew->setPeerAddress(invitation.opponentAddress);
    msg->send();
    delete msg;
    grantedInvitation = invitation;
}

void InternetGameCenter::sendGameAcceptInvitation(FloboGameInvitation &invitation)
{
    opponentName = invitation.opponentName;
    Message *msg = m_igpmbox->createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", FLOBO_IGP_GAME_ACCEPT);
    msg->addString("ORGNAME", name);
    msg->addString("DSTNAME", invitation.opponentName);
    Dirigeable *dirNew = dynamic_cast<Dirigeable *>(msg);
    dirNew->setPeerAddress(invitation.opponentAddress);
    msg->send();
    delete msg;
    if (tryNatTraversal)
        gameGrantedStatus = GAMESTATUS_STARTTRAVERSAL;
    else
        gameGrantedStatus = GAMESTATUS_GRANTED_IGP;
    grantedInvitation = invitation;
}

void InternetGameCenter::grantGameToMBox(MessageBox &thembox)
{
    setStatus(PEER_PLAYING);
    for (int i = 0, j = listeners.size() ; i < j ; i++) {
        listeners[i]->onGameGrantedWithMessagebox(&thembox, grantedInvitation);
    }
}

void InternetGameCenter::sendGameCancelInvitation(FloboGameInvitation &invitation)
{
    Message *msg = m_igpmbox->createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", FLOBO_IGP_GAME_CANCEL);
    msg->addString("ORGNAME", name);
    msg->addString("DSTNAME", invitation.opponentName);
    Dirigeable *dirNew = dynamic_cast<Dirigeable *>(msg);
    dirNew->setPeerAddress(invitation.opponentAddress);
    msg->send();
    delete msg;
}

void InternetGameCenter::idle()
{
    switch (gameGrantedStatus) {
        case GAMESTATUS_STARTTRAVERSAL:
            // The game has been accepted, negociation is pending
            for (int i = 0, j = listeners.size() ; i < j ; i++) {
                listeners[i]->onGameAcceptedNegociationPending(grantedInvitation);
            }
            m_p2pSocket.reset(new DatagramSocket());
            m_p2pSocket->connect(SocketAddress(hostName), portNum);
            m_p2pmbox.reset(new FPInternetP2PMessageBox(m_p2pSocket.get()));
            //printf("grantedAddr:%d\n", static_cast<IgpMessage::IgpPeerAddressImpl *>(grantedInvitation.opponentAddress.getImpl())->getIgpIdent());

            int initiatorIgpIdent, guestIgpIdent;
            if (grantedInvitation.initiatorAddress == grantedInvitation.opponentAddress) {
                // The opponent invited me
                initiatorIgpIdent = static_cast<IgpPeerAddressImpl *>(grantedInvitation.initiatorAddress.getImpl())->getIgpIdent();
                guestIgpIdent = static_cast<IgpPeerAddressImpl *>(m_igpmbox->getSelfAddress().getImpl())->getIgpIdent();
            }
            else {
                // I invited the opponent
                initiatorIgpIdent = static_cast<IgpPeerAddressImpl *>(grantedInvitation.initiatorAddress.getImpl())->getIgpIdent();
                guestIgpIdent = static_cast<IgpPeerAddressImpl *>(grantedInvitation.opponentAddress.getImpl())->getIgpIdent();
            }
            p2pPunchName = String("punch:") + initiatorIgpIdent + "vs" + guestIgpIdent + ":" + grantedInvitation.gameRandomSeed;
            m_p2pNatTraversal.reset(new NatTraversal(*m_p2pmbox));
            m_p2pNatTraversal->punch(p2pPunchName);
            gameGrantedStatus = GAMESTATUS_WAITTRAVERSAL;
            break;
        case GAMESTATUS_WAITTRAVERSAL:
            m_p2pNatTraversal->idle();
            if (m_p2pNatTraversal->hasFailed()) {
                m_p2pNatTraversal.reset();
                m_p2pmbox.reset();
                gameGrantedStatus = GAMESTATUS_GRANTED_IGP;
                GTLogTrace("NAT traversal failed, falling back to IGP");
            }
            else if (m_p2pNatTraversal->hasSucceeded()) {
                m_p2pNatTraversal.reset();
                gameGrantedStatus = GAMESTATUS_GRANTED_P2P;
                GTLogTrace("NAT traversal succeeded, going peer-to-peer");
            }
            break;
        case GAMESTATUS_GRANTED_P2P:
            grantGameToMBox(*m_p2pmbox);
            gameGrantedStatus = GAMESTATUS_IDLE;
            break;
        case GAMESTATUS_GRANTED_IGP:
            m_igpmbox->bind(grantedInvitation.opponentAddress);
            grantGameToMBox(*m_igpmbox);
            gameGrantedStatus = GAMESTATUS_IDLE;
            break;
        case GAMESTATUS_IDLE:
        default:
            break;
    }
    m_igpmbox->idle();
    double time_ms = getTimeMs();
    if ((time_ms - lastAliveMessage) >= timeMsBetweenTwoAliveMessages) {
        sendAliveMessage();
        lastAliveMessage = time_ms;
    }
    NetGameCenter::idle();
}

void InternetGameCenter::setStatus(int status)
{
    this->status = status;
    sendAliveMessage();
}

String InternetGameCenter::getSelfName()
{
    return name;
}

String InternetGameCenter::getOpponentName()
{
    return opponentName;
}

bool InternetGameCenter::isConnected() const
{
    return (m_igpmbox->isConnected()) && (m_isAccepted);
}

bool InternetGameCenter::isDenied() const
{
    return (m_igpmbox->isConnected()) && (m_isDenied);
}

void InternetGameCenter::punch()
{
    int prevBound = m_igpmbox->getBound();
    m_igpmbox->bind(1);
    Message *msg = m_igpmbox->createMessage();
    msg->addBoolProperty("RELIABLE", true);
    msg->addInt("CMD", FLOBO_IGP_NAT_TRAVERSAL);
    msg->send();
    delete msg;
    m_igpmbox->bind(prevBound);
}

void InternetGameCenter::onMessage(Message &msg)
{
    //printf("Cool, un msg!\n");
    try {
        if (!msg.hasInt("CMD"))
            return;
        switch (msg.getInt("CMD")) {
            case FLOBO_IGP_ACCEPT:
                m_isAccepted = true;
                break;
            case FLOBO_IGP_DENY:
                m_isDenied = true;
                m_denyString = msg.getString("MSG");
                if (msg.hasString("MSG_MORE"))
                    m_denyStringMore = msg.getString("MSG_MORE");
                else
                    m_denyStringMore = "";
                break;
            case FLOBO_IGP_CHAT:
                for (int i = 0, j = listeners.size() ; i < j ; i++) {
                    listeners[i]->onChatMessage(msg.getString("NAME"), msg.getString("MSG"));
                }
                break;
            case FLOBO_IGP_CONNECT:
            {
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                int rank = 0;
                if (msg.hasInt("RANK"))
                    rank = msg.getInt("RANK");
                PeerAddress peerAddress = dir.getPeerAddress("ADDR");
                PeerAddress selfAddress = m_igpmbox->getSelfAddress();
                connectPeer(peerAddress, msg.getString("NAME"), msg.getInt("STATUS"), rank, peerAddress == selfAddress);
            }
                break;
            case FLOBO_IGP_DISCONNECT:
            {
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                disconnectPeer(dir.getPeerAddress("ADDR"), msg.getString("NAME"));
            }
                break;
            case FLOBO_IGP_STATUSCHANGE:
            {
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                int rank = 0;
                if (msg.hasInt("RANK"))
                    rank = msg.getInt("RANK");
                connectPeer(dir.getPeerAddress("ADDR"), msg.getString("NAME"), msg.getInt("STATUS"), rank);
            }
                break;
            case FLOBO_IGP_GAME_REQUEST:
            {
                //printf("Une partie contre %s?\n", (const char *)msg.getString("ORGNAME"));
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                FloboGameInvitation invitation;
                invitation.initiatorAddress = dir.getPeerAddress();
                invitation.opponentAddress = dir.getPeerAddress();
                invitation.opponentName = msg.getString("ORGNAME");
                if (msg.hasInt("RNDSEED"))
                    invitation.gameRandomSeed = msg.getInt("RNDSEED");
                else
                    invitation.gameRandomSeed = 0; // When there is no seed, fall back to 0 (better than crashing)
                if (msg.hasInt("SPEED"))
                    invitation.gameSpeed = msg.getInt("SPEED");
                else
                    invitation.gameSpeed = 1; // When there is no seed, fall back to 1 (better than crashing)
                receivedGameInvitation(invitation);
            }
                break;
            case FLOBO_IGP_GAME_ACCEPT:
            {
                //printf("%s accepte la partie!\n", (const char *)msg.getString("ORGNAME"));
    	        setStatus(PEER_PLAYING);
                Dirigeable &dir = dynamic_cast<Dirigeable &>(msg);
                if (!(grantedInvitation.opponentAddress == dir.getPeerAddress()))
                    break;
                if (tryNatTraversal)
                    gameGrantedStatus = GAMESTATUS_STARTTRAVERSAL;
                else
                    gameGrantedStatus = GAMESTATUS_GRANTED_IGP;
            }
                break;
            case FLOBO_IGP_GAME_CANCEL:
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

