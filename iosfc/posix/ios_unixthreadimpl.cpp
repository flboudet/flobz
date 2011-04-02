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

#include "ios_unixthreadimpl.h"
#include "ios_exception.h"

namespace ios_fc {

void UnixThreadImpl::threadExit(void *data)
{
    UnixThreadImpl *target = (UnixThreadImpl*)data;
	target->threadEndFunction();
}

void * UnixThreadImpl::execThreadInObject(void *data)
{
	UnixThreadImpl *target = (UnixThreadImpl*)data;
	pthread_cleanup_push(&threadExit, data);
	target->threadStartFunction();
	pthread_cleanup_pop(1);
	return NULL;
}

UnixThreadImpl::~UnixThreadImpl()
{
}

void UnixThreadImpl::launch()
{
    if (pthread_create(&threadRef, NULL, execThreadInObject, (void *)this) != 0)
        throw Exception("Thread creation error!");
}

void UnixThreadImpl::cancel()
{
#ifndef ANDROID
    pthread_cancel(threadRef);
#endif
}

void UnixThreadImpl::detach()
{
    pthread_detach(threadRef);
}

void UnixThreadImpl::join()
{
    void *result;
    pthread_join(threadRef, &result);
}

ThreadImpl *UnixThreadFactory::createThread()
{
    return new UnixThreadImpl();
}

UnixThreadFactory unixThreadFactory;

ThreadFactory *Thread::factory = &unixThreadFactory;

}


