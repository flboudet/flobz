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

#include "ios_win32socketimpl.h"
#include "ios_exception.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

namespace ios_fc {

void Win32SocketImpl::create(const String hostName, int portID)
{
    struct hostent *hp;
    /* go find out about the desired host machine */
    if ((hp = gethostbyname(hostName)) == 0) {
        //if ((hp = gethostbyaddr(hostName, 32, 0)) == 0)
        throw Exception("IosSocket: gethostbyname error");
    }

    /* fill in the socket structure with host information */
    memset(&pin, 0, sizeof(pin));
    pin.sin_family = AF_INET;
    pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
    pin.sin_port = htons(portID);

    /* grab an Internet domain socket */
    if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        throw Exception("IosSocket: Socket creation failed");
    }

    /* connect to PORT on HOST */
    if (connect(socketFd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
        throw Exception("IosSocket: Socket connection failed");
    }
    inputStream = new SocketInputStream(socketFd);
    outputStream = new SocketOutputStream(socketFd);
}

Win32SocketImpl::~Win32SocketImpl()
{
	delete inputStream;
	delete outputStream;
	closesocket(socketFd);
}

void Win32SocketImpl::socketReceive(void *buffer, int size)
{
	int opResult = recv(socketFd, (char *)buffer, size, 0);
	if ((opResult == 0) && (size > 0)) {
		throw Exception("IosSocket: Socket disconnected");
	}
	else if (opResult < 0) {
		throw Exception("IosSocket: Socket receive error");
	}
}

void Win32SocketImpl::socketSend(const void *buffer, int size)
{
	int opResult = send(socketFd, (char *)buffer, size, 0);
	if ((opResult == 0) && (size > 0)) {
		throw Exception("IosSocket: Socket disconnected");
	}
	else if (opResult < 0) {
		throw Exception("IosSocket: Socket send error");
	}
}

InputStream *Win32SocketImpl::getInputStream()
{
	return inputStream;
}

OutputStream *Win32SocketImpl::getOutputStream()
{
	return outputStream;
}

/*
 * Input stream
 */

Win32SocketImpl::SocketInputStream::SocketInputStream(SOCKET fdesc)
{
	socketFd = fdesc;
}

int Win32SocketImpl::SocketInputStream::streamAvailable()
{
	u_long result = 0;
	if (ioctlsocket(socketFd, FIONREAD, &result) == -1) {
		throw Exception("IosSocketStream: ioctl error");
	}
	return result;
}

int Win32SocketImpl::SocketInputStream::streamRead(VoidBuffer buffer, int size)
{
	int opResult = recv(socketFd, (char *)(buffer.ptr()), size, 0);
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

Win32SocketImpl::SocketOutputStream::SocketOutputStream(SOCKET fdesc)
{
	socketFd = fdesc;
}

int Win32SocketImpl::SocketOutputStream::streamWrite(VoidBuffer buffer, int size)
{
	int opResult = send(socketFd, (char *)(buffer.ptr()), size, 0);
	if ((opResult == 0) && (size > 0)) {
		throw Exception("IosSocket: Socket disconnected");
	}
	else if (opResult < 0) {
		throw Exception("IosSocket: Socket send error");
	}
    return opResult;
}

SocketImpl * Win32SocketFactory::createSocket()
{
    return new Win32SocketImpl();
}

Win32SocketFactory win32SocketFactory;

SocketFactory *Socket::factory = &win32SocketFactory;

}
