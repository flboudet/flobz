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
#include <stdlib.h>
#include "ios_thread.h"
#include "ios_exception.h"


namespace ios_fc {

    void ThreadImpl::setOwner(Thread *owner)
    {
        this->owner = owner;
    }

    void ThreadImpl::threadStartFunction()
    {
        owner->startFunction();
    }

    void ThreadImpl::threadEndFunction()
    {
        if (owner != NULL)
            owner->endFunction();
        else
            delete(this);
    }


    ThreadImpl *ThreadFactory::proceedCreation(Thread *owner)
    {
        ThreadImpl *result = createThread();
        result->setOwner(owner);
        return result;
    }


    Thread::Thread() : isSelfContained(true), attachedRunnable(NULL), threadRunning(false)
    {
        impl = factory->proceedCreation(this);
    }

    Thread::Thread(Runnable *runnable) : isSelfContained(false), attachedRunnable(runnable), threadRunning(false)
    {
        impl = factory->proceedCreation(this);
    }

    Thread::~Thread()
    {
        if (threadRunning)
            impl->setOwner(NULL);
        else
            delete impl;
    }

    void Thread::launch()
    {
        if (!threadRunning) {
            impl->launch();
            threadRunning = true;
        }
        else throw Exception("Thread already running...");
    }

    void Thread::cancel()
    {
        if (!threadRunning)
            throw Exception("Thread was not running...");
        impl->cancel();
    }

    void Thread::detach()
    {
        if (!threadRunning)
            throw Exception("Thread was not running...");
        impl->detach();
    }

    void Thread::join()
    {
        if (!threadRunning)
            throw Exception("Thread was not running...");
        impl->join();
    }

    bool Thread::isRunning() const
    {
        return threadRunning;
    }

    void Thread::startFunction()
    {
        threadRunning = true;
        if (isSelfContained)
            run();
        else
            attachedRunnable->run();
    }

    void Thread::endFunction()
    {
        threadRunning = false;
    }

}
