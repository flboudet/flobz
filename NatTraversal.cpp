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

#include "NatTraversal.h"

NatTraversal::NatTraversal(UDPMessageBoxBase &udpmbox, double punchInfoTimeout, double strategyTimeout)
  : udpmbox(udpmbox), igpmbox(new IgpMessageBox(&udpmbox)), currentStrategy(TRY_NONE), punchInfoTimeout(punchInfoTimeout), strategyTimeout(strategyTimeout), receivedGarbage(0), udpSocketAddress("127.0.0.1"), igpServerSocketAddress(udpmbox.getDatagramSocket()->getConnectedAddress()), igpServerPortNum(udpmbox.getDatagramSocket()->getConnectedPortNum())
{
    igpmbox->addListener(this);
}

NatTraversal::~NatTraversal()
{
    if (igpmbox != NULL) {
        delete igpmbox;
        udpmbox.getDatagramSocket()->disconnect();
    }
    udpmbox.removeListener(this);
}

void NatTraversal::punch(const String punchPoolName)
{
    this->punchPoolName = punchPoolName;
    int prevBound = igpmbox->getBound();
    igpmbox->bind(1);
    Message *punchMsg = igpmbox->createMessage();
    String localSocketAddress = udpmbox.getDatagramSocket()->getSocketAddress().asString();
    int localPortNum = udpmbox.getDatagramSocket()->getSocketPortNum();

    punchMsg->addInt("CMD", PUYO_IGP_NAT_TRAVERSAL);
    punchMsg->addString("PPOOL", punchPoolName);
    punchMsg->addString("LSOCKADDR", localSocketAddress);
    punchMsg->addInt("LPORTNUM", localPortNum);
    punchMsg->addBoolProperty("RELIABLE", true);
    punchMsg->send();
    delete punchMsg;
    igpmbox->bind(prevBound);
    timeToPunchInfo = getTimeMs() + punchInfoTimeout;
    gettingPunchInfo = true;
}

void NatTraversal::idle()
{
    //printf("En ce moment, je suis associee au port %d\n", udpmbox.getDatagramSocket()->getSocketPortNum());
    double currentTime = getTimeMs();
    if (gettingPunchInfo) {
        igpmbox->idle();
        if (timeToPunchInfo < currentTime) {
            currentStrategy = FAILED;
        }
    }
    else {
        switch (currentStrategy) {
            case TRY_NONE: {
                // Switch to TRY_PUBLICADDR
                printf("Trying the TRY_PUBLICADDR strategy\n");
                udpSocketAddress = SocketAddress(peerAddressString);
                udpSocketPortNum = peerPortNum;
                //udpmbox.getDatagramSocket()->connect(sockPubAddr, peerPortNum);
                udpPeerAddress = udpmbox.createPeerAddress(udpSocketAddress, udpSocketPortNum);

                udpmbox.addListener(this);
                currentStrategy = TRY_PUBLICADDR;
                timeToNextStrategy = currentTime + strategyTimeout;
                break;
            }
            case TRY_PUBLICADDR:
                if (timeToNextStrategy < currentTime) {
                    // Switch to TRY_PUBLICADDR_NEXTPORT
                    printf("Trying the TRY_PUBLICADDR_NEXTPORT strategy\n");
                    udpSocketAddress = SocketAddress(peerAddressString);
                    udpSocketPortNum = peerPortNum + 1;
                    //udpmbox.getDatagramSocket()->connect(sockPubAddr, peerPortNum + 1);
                    udpPeerAddress = udpmbox.createPeerAddress(udpSocketAddress, udpSocketPortNum);
                    currentStrategy = TRY_PUBLICADDR_NEXTPORT;
                    timeToNextStrategy = currentTime + strategyTimeout;
                }
                break;
            case TRY_PUBLICADDR_NEXTPORT:
                if (timeToNextStrategy < currentTime) {
                    // Switch to TRY_LOCALADDR
                    printf("Trying the TRY_LOCALADDR strategy\n");
                    udpSocketAddress = SocketAddress(peerLocalAddressString);
                    udpSocketPortNum = peerLocalPortNum;
                    //udpmbox.getDatagramSocket()->connect(sockLocalAddr, peerLocalPortNum);
                    udpPeerAddress = udpmbox.createPeerAddress(udpSocketAddress, udpSocketPortNum);
                    currentStrategy = TRY_LOCALADDR;
                    timeToNextStrategy = currentTime + strategyTimeout;
                }
                break;
            case TRY_LOCALADDR:
                if (timeToNextStrategy < currentTime) {
                    // Switch to FAILED
                    printf("Failed to establish NAT traversal\n");
                    currentStrategy = FAILED;
                }
                break;
            case SYNCING:
                if (timeToNextStrategy < currentTime) {
                    // Switch to FAILED
                    printf("Failed to establish NAT traversal (timeout on syncing)\n");
                    currentStrategy = FAILED;
                }
                break;
            default:
                break;
        }
        sendGarbageMessage();
        udpmbox.idle();
    }
}

void NatTraversal::onMessage(Message &msg)
{
    if (! msg.hasInt("CMD"))
        return;
    switch (msg.getInt("CMD")) {
        case PUYO_IGP_NAT_TRAVERSAL: {
            peerAddressString = msg.getString("SOCKADDR");
            peerLocalAddressString = msg.getString("LSOCKADDR");
            peerPortNum = msg.getInt("PORTNUM");
            peerLocalPortNum = msg.getInt("LPORTNUM");
            printf("Peer:    %s:%d\n", (const char *)peerAddressString, peerPortNum);
            printf("Peer(L): %s:%d\n", (const char *)peerLocalAddressString, peerLocalPortNum);

            // Destroy the igp messagebox
            //delete igpmbox;
            //igpmbox = NULL;
            gettingPunchInfo = false;
            udpmbox.getDatagramSocket()->disconnect();

            // Connect the udp messagebox to the local address of the peer
            /*SocketAddress sockLocAddr(peerLocalAddressString);
            udpmbox.addListener(this);
            udpmbox.getDatagramSocket()->connect(sockLocAddr, peerPortNum);
            PeerAddress peerAddr = udpmbox.createPeerAddress(sockLocAddr, peerPortNum);
            udpmbox.bind(peerAddr);*/
            break;
        }
        case PUYO_IGP_NAT_TRAVERSAL_GARBAGE:
            //printf("Garbage msg received: %s (%d)\n", (const char *)(msg.getString("GARBAGE")), msg.getInt("RCV"));
            receivedGarbage++;
            if ((msg.getInt("RCV") > 0) && (currentStrategy != SYNCING) && (currentStrategy != SUCCESS) && (currentStrategy != FAILED)) {
                currentStrategy = SYNCING;
                timeToNextStrategy = getTimeMs() + strategyTimeout;
                sendSyncMessage();
            }
            break;
        case PUYO_IGP_NAT_TRAVERSAL_SYNC:
             currentStrategy = SUCCESS;
             printf("Punching is a success!\n");
             // Destroy the igp messagebox
            delete igpmbox;
            igpmbox = NULL;
             udpmbox.getDatagramSocket()->connect(udpSocketAddress, udpSocketPortNum);
             break;
        default:
            break;
    }
}

void NatTraversal::sendGarbageMessage()
{
    if (udpPeerAddress == udpmbox.createPeerAddress(udpmbox.getDatagramSocket()->getSocketAddress(), udpmbox.getDatagramSocket()->getSocketPortNum()))
        printf("Envoi de messages a moi-meme!!!!!");
    //printf("Envoi garbage\n");
    bool connectedState = udpmbox.getDatagramSocket()->getConnected();
    if (connectedState)
      udpmbox.getDatagramSocket()->disconnect();
    //PeerAddress prevBound = udpmbox.getBound();
    //udpmbox.bind(udpPeerAddress);
    Message *garbMsg = udpmbox.createMessage();

    garbMsg->addInt("CMD", PUYO_IGP_NAT_TRAVERSAL_GARBAGE);
    garbMsg->addString("GARBAGE", "GNU is not free software");
    garbMsg->addInt("RCV", receivedGarbage);

    Dirigeable *dirigeableMsg = dynamic_cast<Dirigeable *>(garbMsg);
    dirigeableMsg->setPeerAddress(udpPeerAddress);
    garbMsg->send();
    delete garbMsg;
    //udpmbox.bind(prevBound);
    if (connectedState)
      udpmbox.getDatagramSocket()->connect(igpServerSocketAddress, igpServerPortNum);
}

void NatTraversal::sendSyncMessage()
{
    printf("send SYNC\n");
    //udpmbox.getDatagramSocket()->connect(igpServerSocketAddress, igpServerPortNum);
    int prevBound = igpmbox->getBound();
    igpmbox->bind(1);
    Message *message = igpmbox->createMessage();
    message->addInt("CMD", PUYO_IGP_NAT_TRAVERSAL_SYNC);
    message->addString("PPOOL", punchPoolName);
    message->addBoolProperty("RELIABLE", true);
    message->send();
    delete message;
    igpmbox->bind(prevBound);
}

