/* Ultimate Othello 1678
 * Copyright (C) 2002  Florent Boudet <flobo@ifrance.com>
 * iOS Software <http://www.ios-software.com>
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

#ifndef _IOS_VECTOR_H
#define _IOS_VECTOR_H

#include "ios_memory.h"

namespace ios_fc {
    
template <typename T>
class Vector : public AdvancedBuffer<T *>
{
    public:
        Vector<T> dup() const {
            Vector<T> buf;
            for (int i=0; i<this->size(); ++i) buf.add(this->get(i));
            return buf;
        }
};

template <typename T>
class SelfVector : public AdvancedBuffer<T *>
{
    public:
    ~SelfVector() {
        for (int i=0; i < this->size(); ++i)
            delete this->get(i);
    }
    
    inline int  remove(const T *t)
    {
      // WARNING: I do this cause I know AdvancedBuffer's implementation...
      // (this is bad)
      int i = AdvancedBuffer<T*>::remove(t);
      delete this->get(this->size());
      return i;
    }
    
    inline void remove()
    {
      AdvancedBuffer<T*>::remove();
      // WARNING: I do this cause I know AdvancedBuffer's implementation...
      // (this is bad)
      delete this->get(this->size());
    }
};

};

#endif // _IOS_VECTOR_H
