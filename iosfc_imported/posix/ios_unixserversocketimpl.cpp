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
 
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include "ios_unixserversocketimpl.h"
#include "ios_unixsocketimpl.h"
#include "ios_exception.h"

#define DONT_USE_CONFIG_H
#ifndef DONT_USE_CONFIG_H
#include <config.h>
#else
#ifdef NO_SOCKLEN_T
#define socklen_t int
#endif
#endif


namespace ios_fc {

UnixServerSocketImpl::UnixServerSocketImpl()
{
}

UnixServerSocketImpl::~UnixServerSocketImpl()
{
    fprintf(stderr, "Server Socket shutdown\n");
    int err = shutdown(sd, 2);
    if (err != 0)
        fprintf(stderr, "Server Socket shutdown failed\n");
    close(sd);
}

void UnixServerSocketImpl::create(int port)
{
    this->port = port;
    createServerSocket();
}

void UnixServerSocketImpl::createServerSocket()
{
    /* get an internet domain socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        throw Exception("Socket creation error");
    }
    
    /* complete the socket structure */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);
    
    /* bind the socket to the port number */
    if (bind(sd, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
        throw Exception("Socket binding error");
    }
    
    /* show that we are willing to listen */
    if (listen(sd, 5) == -1) {
        throw Exception("Socket listening error");
    }
}

SocketImpl *UnixServerSocketImpl::acceptClientImpl()
{
    int sd_client;
    if ((sd_client = accept(sd, (struct sockaddr *)  &pin, (socklen_t *)(&addrlen))) == -1) {
        throw Exception("Socket waiting client error");
    }
    UnixSocketImpl *newImpl = new UnixSocketImpl();
    newImpl->create(sd_client);
    return newImpl;
}

ServerSocketImpl * UnixServerSocketFactory::createServerSocket()
{
    return new UnixServerSocketImpl();
}

UnixServerSocketFactory unixServerSocketFactory;

ServerSocketFactory *ServerSocket::factory = &unixServerSocketFactory;

};
