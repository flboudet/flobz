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
 
#ifndef _IOSSOCKETADDRESS
#define _IOSSOCKETADDRESS

#include "ios_memory.h"

namespace ios_fc {

    class SocketAddressImpl {
    public:
        SocketAddressImpl() : usage(0) {}
        inline int getUsage() const { return usage; }
        inline void incrementUsage() { usage++; }
        inline void decrementUsage() { usage--; }
        virtual ~SocketAddressImpl() {}
    private:
        int usage;
    };

    class SocketAddressFactory {
    public:
        virtual SocketAddressImpl * createSocketAddress(String hostName) = 0;
    };

    class SocketAddress {
    public:
        SocketAddress(String hostName) { impl = factory->createSocketAddress(hostName); impl->incrementUsage(); }
        SocketAddress(SocketAddressImpl *impl) { this->impl = impl; impl->incrementUsage(); }
        SocketAddress(const SocketAddress &s) : impl(s.impl) { impl->incrementUsage(); }
        ~SocketAddress() {
            impl->decrementUsage();
            if (impl->getUsage() == 0)
                delete impl;
        }
        SocketAddressImpl *getImpl() const { return impl; }
        static void setFactory(SocketAddressFactory *factory) { SocketAddress::factory = factory; }
    private:
        static SocketAddressFactory *factory;
        SocketAddressImpl *impl;
    };

}

#endif // _IOSSOCKETADDRESS

