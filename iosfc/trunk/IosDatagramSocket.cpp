/* Ultimate Othello 1678
 * Copyright (C) 2002  Florent Boudet <flobo@ifrance.com>
 * iOS Software <http://ios.free.fr>
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

#include <stdlib.h>
#include <string.h>
#include "IosDatagramSocket.h"

bool IosDatagramSocket::isInitialized = false;

void IosDatagramSocket::initSDLnet()
{
    SDLNet_Init();
    IosDatagramSocket::isInitialized = true;
}

IosDatagramSocket::IosDatagramSocket(const char *hostName, const int localPortID, const int remotePortID)
{
    if ( ! IosDatagramSocket::isInitialized)
        initSDLnet();
    IPaddress address;
    if (SDLNet_ResolveHost(&address, hostName, remotePortID) == -1) {
        fprintf(stderr, "Name resolution failed!\n");
    }
    udpsock = SDLNet_UDP_Open(localPortID);
    if (udpsock == NULL) {
        fprintf(stderr, "Socket creation failed!\n");
    }
    SDLNet_UDP_Bind(udpsock, -1, &address);
    innerpacket = SDLNet_AllocPacket(1024);
}

IosDatagramSocket::IosDatagramSocket(const int localPortID)
{
    if ( ! IosDatagramSocket::isInitialized)
        initSDLnet();
    udpsock = SDLNet_UDP_Open(localPortID);
    innerpacket = SDLNet_AllocPacket(1024);
}

IosDatagramSocket::~IosDatagramSocket()
{
    SDLNet_UDP_Close(udpsock);
    SDLNet_FreePacket(innerpacket);
}

void IosDatagramSocket::socketSend(const void *buffer, int size)
{
    //SDLNet_ResolveHost(&(innerpacket->address), "192.168.1.10", 0);
    innerpacket->channel = -1;
    memcpy(innerpacket->data, buffer, size);
    innerpacket->len = size;
    SDLNet_UDP_Send(udpsock, 0, innerpacket);
}

void IosDatagramSocket::socketReceive(void *buffer, int &size)
{
    size = 0;
    int result = SDLNet_UDP_Recv(udpsock, innerpacket);
    if (result == 1) {
        memcpy(buffer, innerpacket->data, innerpacket->len);
        size = innerpacket->len;
    }
}

