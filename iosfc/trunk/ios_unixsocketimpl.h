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
 
#ifndef _UNIXSOCKETIMPL
#define _UNIXSOCKETIMPL

#include "ios_socket.h"
#include "ios_unixselectorimpl.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

namespace ios_fc {

class UnixSocketImpl : public SocketImpl, public UnixSelectableImpl {
public:
	virtual void create(String hostName, int portID);
        void create(int fd);
	virtual ~UnixSocketImpl();

	InputStream *getInputStream();
	OutputStream *getOutputStream();
        
	void socketSend(const void *buffer, int size);
	void socketReceive(void *buffer, int size);
        
        SelectableImpl *getSelectableImpl() { return this; }
        int getFd() { return socketID; }

private:

	class SocketInputStream : public InputStream {
	public:
		SocketInputStream(int fdesc);
		int streamAvailable();
		int streamRead(VoidBuffer buffer, int size);
	private:
			int socketID;
	};
    
	class SocketOutputStream : public OutputStream {
	public:
		SocketOutputStream(int fdesc);
		int streamWrite(VoidBuffer buffer, int size);
	private:
		int socketID;
	};
    
	SocketInputStream *inputStream;
	SocketOutputStream *outputStream;
	int socketID;
	struct sockaddr_in pin;
};

class UnixSocketFactory : public SocketFactory {
public:
    virtual SocketImpl * createSocket();
};

};
#endif // _UNIXSOCKETIMPL

