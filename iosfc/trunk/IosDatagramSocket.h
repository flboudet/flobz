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
 
#ifndef _IOSDATAGRAMSOCKET
#define _IOSDATAGRAMSOCKET

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "SDL.h"
#include "SDL_net.h"

class IosDatagramSocket {
public:
	IosDatagramSocket(const char *hostName, const int localPortID,  const int remotePortID);
	IosDatagramSocket(const int localPortID);
	virtual ~IosDatagramSocket();
	void socketSend(const void *buffer, int size);
	void socketReceive(void *buffer, int &size);
private:
	static bool isInitialized;
    static void initSDLnet();
    UDPsocket udpsock;
    UDPpacket *innerpacket;
};

#endif // _IOSDATAGRAMSOCKET

