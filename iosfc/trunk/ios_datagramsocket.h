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

#include "ios_stream.h"
#include "ios_memory.h"
#include "ios_selectable.h"
#include "ios_socketaddress.h"

namespace ios_fc {

    /// @brief Manage a Datagram sent by a DatagramSocket.
    class Datagram {
    public:
        Datagram(SocketAddress address, int portNum, VoidBuffer message, int size) : message(message), address(address), portNum(portNum), size(size) {}
        inline VoidBuffer getMessage() const { return message; }
        inline SocketAddress getAddress() const { return address; }
        inline int getPortNum() const { return portNum; }
        inline int getSize() const { return size; }
    private:
        VoidBuffer message;
        SocketAddress address;
        int portNum;
        int size;
    };
    
    /// @brief Interface defining Datagram implementation.
    class DatagramSocketImpl {
    public:
        DatagramSocketImpl() {}
        virtual ~DatagramSocketImpl() {}
        virtual void create(int localPortNum) = 0;
        virtual void send(Datagram &sendDatagram) = 0;
        virtual Datagram receive(VoidBuffer buffer) = 0;
        virtual SelectableImpl *getSelectableImpl() = 0;
        virtual SocketAddress getBroadcastAddress() const = 0;
        virtual SocketAddress getSocketAddress() const = 0;
        virtual int getSocketPortNum() const = 0;
        virtual int available() const = 0;
        virtual void connect(SocketAddress addr, int portNum) = 0;
        virtual void disconnect() = 0;
        virtual bool getConnected() = 0;
        virtual void joinGroup(SocketAddress groupAddress) = 0;
        virtual SocketAddress getConnectedAddress() = 0;
        virtual int getConnectedPortNum() = 0;
    };
    
    /// @brief Interface of a factory for DatagramSocketImpl.
    class DatagramSocketFactory {
    public:
        virtual DatagramSocketImpl * createDatagramSocket() = 0;
    };

    /// @brief Manage an UDP socket, for Datagram based network communication.
    class DatagramSocket : public Selectable {
    public:
//        DatagramSocket(const String hostName, int localPortNum, int remotePortNum);
        DatagramSocket(int localPortNum);
        DatagramSocket();
        DatagramSocket(DatagramSocketImpl *impl);
        virtual ~DatagramSocket();
        virtual void send(Datagram &sendDatagam);
        virtual Datagram receive(VoidBuffer buffer);
        virtual int available() const;
        virtual void joinGroup(SocketAddress groupAddress);
        virtual void connect(SocketAddress addr, int portNum);
        virtual void disconnect();
        virtual bool getConnected();
        virtual SocketAddress getConnectedAddress();
        virtual int getConnectedPortNum();
        SocketAddress getBroadcastAddress() const;
        SocketAddress getSocketAddress() const;
        int getSocketPortNum() const;
        static void setFactory(DatagramSocketFactory *factory) { DatagramSocket::factory = factory; }
        
        // Selectable implementation
        SelectableImpl *getSelectableImpl() const;
        // Datagram socket implementation
        DatagramSocketImpl &getImpl() const { return *impl; }
        
        enum {
            NODEFINEDPORT=-1
        };
    private:
        static DatagramSocketFactory *factory;
        DatagramSocketImpl *impl;
        SelectableImpl *sImpl;

        DatagramSocket(const DatagramSocket&ds) : impl(0),sImpl(0) {}
        DatagramSocket&operator=(const DatagramSocket&ds) {
            impl=0;sImpl=0;
            return *this;
        }
    };
    
}
#endif // _IOSDATAGRAMSOCKET

