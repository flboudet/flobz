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

#include <pthread.h>
#include <errno.h>
#include "ios_mutex.h"
#include "ios_exception.h"

namespace ios_fc {

    class PThreadMutexImpl : public MutexImpl {
    public:
        PThreadMutexImpl() {
            pthread_mutex_init(&m_mutex, NULL);
        }
        virtual ~PThreadMutexImpl() {
            pthread_mutex_destroy(&m_mutex);
        }
        virtual void lock() {
            pthread_mutex_lock(&m_mutex);
        }
        virtual bool trylock() {
            int ret = pthread_mutex_trylock(&m_mutex);
            if (ret == EBUSY)
                return true;
            return false;
        }
        virtual void unlock() {
            pthread_mutex_unlock(&m_mutex);
        }
    private:
        pthread_mutex_t m_mutex;
    };

    class PThreadMutexFactory : public MutexFactory {
    public:
        virtual MutexImpl * createMutex() {
            return new PThreadMutexImpl();
        }
    };

    PThreadMutexFactory pthreadMutexFactory;

    MutexFactory *Mutex::factory = &pthreadMutexFactory;

} // namespace

