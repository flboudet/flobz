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

#include "ios_sdlsocketimpl.h"
#include "ios_exception.h"

namespace ios_fc {

void SdlSocketImpl::create(const String hostName, int portID)
{
	/* go find out about the desired host machine */
	if (SDLNet_ResolveHost(&ip, hostName, portID) == -1) {
        throw Exception("IosSocket: gethostbyname error");
	}

	/* connect */
    tcpsock = SDLNet_TCP_Open(&ip);
	if (!tcpsock) {
		throw Exception("IosSocket: Socket connection failed");
	}
	inputStream = new SocketInputStream(tcpsock);
	outputStream = new SocketOutputStream(tcpsock);
}

SdlSocketImpl::~SdlSocketImpl()
{
    delete inputStream;
    delete outputStream;
	SDLNet_TCP_Close(tcpsock);
}

InputStream *SdlSocketImpl::getInputStream()
{
	return inputStream;
}

OutputStream *SdlSocketImpl::getOutputStream()
{
	return outputStream;
}

/*
 * Input stream
 */

SdlSocketImpl::SocketInputStream::SocketInputStream(TCPsocket &tcpsock) : tcpsock(tcpsock)
{
}

int SdlSocketImpl::SocketInputStream::streamAvailable()
{
	int result = 0;
	return result;
}

int SdlSocketImpl::SocketInputStream::streamRead(VoidBuffer buffer)
{
    int size = buffer.size();
	int opResult = SDLNet_TCP_Recv(tcpsock, buffer.ptr(), size);
	if ((opResult == 0) && (size > 0)) {
		throw Exception("IosSocket: Socket disconnected");
	}
	else if (opResult < 0) {
		throw Exception("IosSocket: Socket receive error");
	}
    return opResult;
}

/*
 * Output stream
 */

SdlSocketImpl::SocketOutputStream::SocketOutputStream(TCPsocket &tcpsock) : tcpsock(tcpsock)
{
}

int SdlSocketImpl::SocketOutputStream::streamWrite(VoidBuffer buffer)
{
    int size = buffer.size();
	int opResult = SDLNet_TCP_Send(tcpsock, buffer.ptr(), size);
	if ((opResult == 0) && (size > 0)) {
		throw Exception("IosSocket: Socket disconnected");
	}
	else if (opResult < 0) {
		throw Exception("IosSocket: Socket send error");
	}
    return opResult;
}

SocketImpl * SdlSocketFactory::createSocket()
{
    return new SdlSocketImpl();
}

SdlSocketFactory sdlSocketFactory;

SocketFactory *Socket::factory = &sdlSocketFactory;

};
