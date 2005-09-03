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

#ifndef _IOSTHREAD
#define _IOSTHREAD

namespace ios_fc {

    class Thread;
    
    class ThreadImpl {
    public:
        ThreadImpl() : owner(NULL) {}
        virtual ~ThreadImpl() {}
        virtual void launch() = 0;
        virtual void cancel() = 0;
        virtual void detach() = 0;
        virtual void join() = 0;
        void setOwner(Thread *owner);
    protected:
        Thread *owner;
        void threadStartFunction();
        void threadEndFunction();
    };
    
    class ThreadFactory {
    public:
        ThreadImpl *proceedCreation(Thread *owner);
        virtual ThreadImpl * createThread() = 0;
    };
    
    class Runnable {
    public:
        virtual void run() = 0;
    };
    
    class Thread {
    public:
        Thread();
        Thread(Runnable *runnable);
        virtual ~Thread();
        virtual void launch();
        virtual void cancel();
        virtual void detach();
        virtual void join();
        virtual bool isRunning() const;
        virtual void run() {}
        
        static void setFactory(ThreadFactory *factory) { Thread::factory = factory; }
        
        void startFunction();
        void endFunction();
    private:
        static ThreadFactory *factory;
        bool isSelfContained;
        Runnable *attachedRunnable;
        volatile bool threadRunning;
        ThreadImpl *impl;
    };

}

#endif // _IOSTHREAD

