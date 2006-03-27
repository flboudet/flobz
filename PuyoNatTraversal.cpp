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

#include "PuyoNatTraversal.h"

PuyoNatTraversal::PuyoNatTraversal(UDPMessageBox &udpmbox, double punchInfoTimeout, double strategyTimeout)
  : udpmbox(udpmbox), igpmbox(new IgpMessageBox(udpmbox)), currentStrategy(TRY_NONE), punchInfoTimeout(punchInfoTimeout), strategyTimeout(strategyTimeout)
{
    igpmbox->addListener(this);
    //printf("GetSocketAddress(): %s\n", (const char *)(mbox->getSocketAddress().asString()));
    //printf("GetSocketPortNum(): %d\n", mbox->getDatagramSocket()->getSocketPortNum());
}

PuyoNatTraversal::~PuyoNatTraversal()
{
    if (igpmbox != NULL) {
        delete igpmbox;
        udpmbox.getDatagramSocket()->disconnect();
    }
    udpmbox.removeListener(this);
}

void PuyoNatTraversal::punch(const String punchPoolName)
{
    int prevBound = igpmbox->getBound();
    igpmbox->bind(1);
    Message *punchMsg = igpmbox->createMessage();
    String localSocketAddress = udpmbox.getSocketAddress().asString();
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
}

void PuyoNatTraversal::idle()
{
    double currentTime = getTimeMs();
    if (igpmbox != NULL) {
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
                SocketAddress sockPubAddr(peerAddressString);
                udpmbox.addListener(this);
                udpmbox.getDatagramSocket()->connect(sockPubAddr, peerPortNum);
                PeerAddress peerPubAddr = udpmbox.createPeerAddress(sockPubAddr, peerPortNum);
                udpmbox.bind(peerPubAddr);
                currentStrategy = TRY_PUBLICADDR;
                timeToNextStrategy = currentTime + strategyTimeout;
                break;
            }
            case TRY_PUBLICADDR:
                if (timeToNextStrategy < currentTime) {
                    // Switch to TRY_PUBLICADDR_NEXTPORT
                    printf("Trying the TRY_PUBLICADDR_NEXTPORT strategy\n");
                    SocketAddress sockPubAddr(peerAddressString);
                    udpmbox.getDatagramSocket()->connect(sockPubAddr, peerPortNum + 1);
                    PeerAddress peerPubAddr = udpmbox.createPeerAddress(sockPubAddr, peerPortNum + 1);
                    udpmbox.bind(peerPubAddr);
                    currentStrategy = TRY_PUBLICADDR_NEXTPORT;
                    timeToNextStrategy = currentTime + strategyTimeout;
                }
                break;
            case TRY_PUBLICADDR_NEXTPORT:
                if (timeToNextStrategy < currentTime) {
                    // Switch to TRY_LOCALADDR
                    printf("Trying the TRY_LOCALADDR strategy\n");
                    SocketAddress sockLocalAddr(peerLocalAddressString);
                    udpmbox.getDatagramSocket()->connect(sockLocalAddr, peerLocalPortNum);
                    PeerAddress peerLocalAddr = udpmbox.createPeerAddress(sockLocalAddr, peerLocalPortNum);
                    udpmbox.bind(peerLocalAddr);
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
            default:
                break;
        }
        sendGarbageMessage();
        udpmbox.idle();
    }
}

void PuyoNatTraversal::onMessage(Message &msg)
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
            delete igpmbox;
            igpmbox = NULL;
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
            printf("Garbage msg received: %s\n", (const char *)(msg.getString("GARBAGE")));
            currentStrategy = SUCCESS;
            break;
        default:
            break;
    }
}

void PuyoNatTraversal::sendGarbageMessage()
{
    //printf("Envoi garbage\n");
    Message *garbMsg = udpmbox.createMessage();
    
    garbMsg->addInt("CMD", PUYO_IGP_NAT_TRAVERSAL_GARBAGE);
    garbMsg->addString("GARBAGE", "Connerie");
    
    garbMsg->send();
    delete garbMsg;
}

