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
 
#ifndef _SDLSOCKETIMPL
#define _SDLSOCKETIMPL

#include "ios_socket.h"
#include "SDL.h"
#include "SDL_net.h"

namespace ios_fc {

class SdlSocketImpl : public SocketImpl {
public:
	virtual void create(String hostName, int portID);
	virtual ~SdlSocketImpl();

	InputStream *getInputStream();
	OutputStream *getOutputStream();

private:

	class SocketInputStream : public InputStream {
	public:
		SocketInputStream(TCPsocket &tcpsock);
		int streamAvailable();
		int streamRead(VoidBuffer buffer);
	private:
        TCPsocket &tcpsock;
	};
    
	class SocketOutputStream : public OutputStream {
	public:
		SocketOutputStream(TCPsocket &tcpsock);
		int streamWrite(VoidBuffer buffer);
	private:
		TCPsocket &tcpsock;
	};
    
	SocketInputStream *inputStream;
	SocketOutputStream *outputStream;
	IPaddress ip;
    TCPsocket tcpsock;
};

class SdlSocketFactory : public SocketFactory {
public:
    virtual SocketImpl * createSocket();
};

};
#endif // _SDLSOCKETIMPL

