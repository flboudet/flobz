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

#include "ios_unixsocketimpl.h"
#include "ios_exception.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

namespace ios_fc {

void UnixSocketImpl::create(const String hostName, int portID)
{
    struct hostent *hp;
    /* go find out about the desired host machine */
    if ((hp = gethostbyname2(hostName, AF_INET)) == 0) {
        //if ((hp = gethostbyaddr(hostName, 32, 0)) == 0)
        throw Exception("IosSocket: gethostbyname error");
    }

    /* fill in the socket structure with host information */
    memset(&pin, 0, sizeof(pin));
    pin.sin_family = AF_INET;
    pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
    pin.sin_port = htons(portID);

    /* grab an Internet domain socket */
    if ((socketID = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        throw Exception("IosSocket: Socket creation failed");
    }

    /* connect to PORT on HOST */
    if (connect(socketID,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
        throw Exception("IosSocket: Socket connection failed");
    }
    inputStream = new SocketInputStream(socketID);
    outputStream = new SocketOutputStream(socketID);
}

void UnixSocketImpl::create(int fd)
{
    socketID = fd;
    inputStream = new SocketInputStream(socketID);
    outputStream = new SocketOutputStream(socketID);
}

UnixSocketImpl::~UnixSocketImpl()
{
	delete inputStream;
	delete outputStream;
	close(socketID);
}

void UnixSocketImpl::socketReceive(void *buffer, int size)
{
	int opResult = read(socketID, buffer, size);
	if ((opResult == 0) && (size > 0)) {
		throw Exception("IosSocket: Socket disconnected");
	}
	else if (opResult < 0) {
		throw Exception("IosSocket: Socket receive error");
	}
}

void UnixSocketImpl::socketSend(const void *buffer, int size)
{
	int opResult = write(socketID, buffer, size);
	if ((opResult == 0) && (size > 0)) {
		throw Exception("IosSocket: Socket disconnected");
	}
	else if (opResult < 0) {
		throw Exception("IosSocket: Socket send error");
	}
}

InputStream *UnixSocketImpl::getInputStream()
{
	return inputStream;
}

OutputStream *UnixSocketImpl::getOutputStream()
{
	return outputStream;
}

/*
 * Input stream
 */

UnixSocketImpl::SocketInputStream::SocketInputStream(int fdesc)
{
	socketID = fdesc;
}

int UnixSocketImpl::SocketInputStream::streamAvailable()
{
	int result = 0;
	if (ioctl(socketID, FIONREAD, &result) == -1) {
		throw Exception("IosSocketStream: ioctl error");
	}
	return result;
}

int UnixSocketImpl::SocketInputStream::streamRead(VoidBuffer buffer, int size)
{
	int opResult = read(socketID, buffer.ptr(), size);
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

UnixSocketImpl::SocketOutputStream::SocketOutputStream(int fdesc)
{
	socketID = fdesc;
}

int UnixSocketImpl::SocketOutputStream::streamWrite(VoidBuffer buffer, int size)
{
	int opResult = write(socketID, buffer.ptr(), size);
	if ((opResult == 0) && (size > 0)) {
		throw Exception("IosSocket: Socket disconnected");
	}
	else if (opResult < 0) {
		throw Exception("IosSocket: Socket send error");
	}
    return opResult;
}

SocketImpl * UnixSocketFactory::createSocket()
{
    return new UnixSocketImpl();
}

UnixSocketFactory unixSocketFactory;

SocketFactory *Socket::factory = &unixSocketFactory;

};
