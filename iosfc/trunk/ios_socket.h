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
 
#ifndef _IOSSOCKET
#define _IOSSOCKET

#include "ios_stream.h"
#include "ios_memory.h"

namespace ios_fc {
    
    typedef InputStream SelectableInputStream;
    typedef OutputStream SelectableOutputStream;
    
    class SocketImpl {
    public:
        SocketImpl() {}
        virtual ~SocketImpl() {}
        virtual void create(const String hostName, int portID) = 0;
        virtual InputStream *getInputStream() = 0;
        virtual OutputStream *getOutputStream() = 0;
    private:
        SelectableInputStream *inputStream;
        SelectableOutputStream *outputStream;
    };
    
    class SocketFactory {
    public:
        virtual SocketImpl * createSocket() = 0;
    };
    
    class Socket {
    public:
        Socket(const String hostName, int portID);
        virtual ~Socket();
        virtual InputStream *getInputStream();
        virtual OutputStream *getOutputStream();
        static void setFactory(SocketFactory *factory) { Socket::factory = factory; }
    private:
        static SocketFactory *factory;
        SocketImpl *impl;
    };
};
#endif // _IOSSOCKET
