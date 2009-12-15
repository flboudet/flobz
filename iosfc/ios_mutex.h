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

#ifndef _IOSMUTEX
#define _IOSMUTEX

namespace ios_fc {

    class Mutex;

    class MutexImpl {
    public:
        virtual ~MutexImpl() {}
        virtual void lock() = 0;
        virtual bool trylock() = 0;
        virtual void unlock() = 0;
    };

    class MutexFactory {
    public:
        virtual MutexImpl * createMutex() = 0;
        virtual ~MutexFactory() {}
    };

    class Mutex {
    public:
        Mutex() {
            m_impl = factory->createMutex();
        }
        virtual ~Mutex() { delete m_impl; }
        virtual void lock()    { m_impl->lock();    }
        virtual bool trylock() { return m_impl->trylock(); }
        virtual void unlock()  { m_impl->unlock();  }
        static void setFactory(MutexFactory *factory) { Mutex::factory = factory; }
    private:
        static MutexFactory *factory;
        MutexImpl *m_impl;
    };

    class Lock {
    public:
        Lock(Mutex &mut)
            : m_mut(mut) { m_mut.lock();   }
        virtual ~Lock()  { m_mut.unlock(); }
    private:
        Mutex &m_mut;
    };

};

#endif // _IOSMUTEX

