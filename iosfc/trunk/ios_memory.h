#ifndef _IOSMEMORY_H
#define _IOSMEMORY_H

/**
 * iosfc::Memory: contains useful memory management class.
 *
 * - Memory:            gives access to malloc, free, realloc and memcpy.
 * - VoidBuffer:        simulate a (void*) pointer. provide garbage collection!
 * - Buffer<T>:         simulate a (T*) buffer using VoidBuffer.
 * - AdvancedBuffer<T>: a vector implemented as a Buffer<T>.
 * - String             a string.
 * 
 * This file is part of the iOS Foundation Classes project.
 *
 * authors:
 *  Jean-Christophe Hoelt <jeko@ios-software.com>
 *  Guillaume Borios      <gyom@ios-software.com>
 *  Florent Boudet       <flobo@ios-software.com>
 *  Michel Metzger -- participate during the developement of Shubunkin!
 *
 * http://www.ios-software.com/
 *
 * Released under the terms of the GNU General Public Licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include <cstdlib>
#include <cstring>
#include <cstdio>

#define DEBUG_MEMORY

//
// important note for users:
// all object from thoses classes must be passed by value.
//

namespace ios_fc {

    /**
     * Overide memory management methods to allow easy debugging.
     */
    class Memory {
		public:
#ifdef DEBUG_MEMORY
            static inline void *malloc  (int size);
            static inline void *calloc  (int size);
            static inline void  free    (void *ptr);
            static inline void *realloc (void *ptr, int size);
            static inline void  memcpy  (void *dest, const void *src, int len);
#else
            static inline void *malloc  (int size)            { return ::malloc(size?size:1); }
            static inline void *calloc  (int size)            { return ::calloc(1,size?size:1); }
            static inline void  free    (void *ptr)           { ::free(ptr); }
            static inline void *realloc (void *ptr, int size) { return ::realloc(ptr, size?size:1); }
            static inline void  memcpy  (void *dest, const void *src, int len) { ::memcpy(dest,src,len); }
#endif
    };

    /**
     * simulate a (void*) pointer. provide garbage collection!
     */
    class VoidBuffer {

        protected:
            struct Ptr {
                void *ptr;
                int size;
                int nb;
            };
#ifdef DEBUG_MEMORY
            inline void init_ptr (int size);
#else
            inline void init_ptr (int size) {
                p       = new Ptr();
                p->ptr  = Memory::malloc(size);
                p->size = size;
                p->nb   = 1;
            }
#endif
			inline void unregPtr() {
                if (!(-- p->nb)) {
                    Memory::free(p->ptr);
                    delete p;
                }
			}

        public:

            /**
             * Build a VoidBuffer with initial size.
             *
             * @param size Initial size.
             */
            inline VoidBuffer (int size = 0) : offset(0) {
                init_ptr(size);
            }

            /**
             * Build a VoidBuffer by duplicating an existing one.
             *
             * @param buf VoidBuffer to duplicate.
             */
            inline VoidBuffer (const VoidBuffer &buf)
                : offset(0)
            {
              init_ptr(buf.size());
              Memory::memcpy(ptr(), buf.ptr(), size());
            }
            
            inline VoidBuffer (VoidBuffer &buf)
                : p(buf.p), offset(0)
            {
                p->nb ++;
            }

            /**
             * Build a VoidBuffer from an non-ios_fc buffer.
             *
             * @param data Pointer to the buffer.
             * @param len  Size of the buffer.
             */
            inline VoidBuffer (const void *data,int len) : offset(0) {
                init_ptr (len);
                Memory::memcpy (p->ptr,data,len);
            }

            /**
             * Gives a copy of a VoidBuffer, sharing the datas.
             *
             * @param vb Buffer to copy.
             */
			inline const VoidBuffer &operator= (VoidBuffer &vb) {
				unregPtr();
				p      = vb.p;
				offset = vb.offset;
				++ p->nb;
				return *this;
			}

            /**
             * Gives a copy of a VoidBuffer, duplicating the datas.
             *
             * @param vb VoidBuffer to duplicate.
             */
			inline const VoidBuffer &operator= (const VoidBuffer &vb) {
                unregPtr();
                init_ptr(vb.size());
                Memory::memcpy(ptr(), vb.ptr(), size());
                return *this;
            }

            /**
             * Destructor.
             */
            inline ~VoidBuffer () {
				unregPtr();
            }

            /**
             * Returns the pointer to the datas.
             */
            inline void *ptr()  const { return (void*)((char*)p->ptr + offset); }

            /**
             * Returns the size of the buffer in byte.
             */
            inline int   size() const { return p->size - offset; }

            /**
             * Changes the size of the buffer.
             *
             * @param size New size of the buffer.
             */
            inline void  realloc (int size) {
                p->size = size + offset;
                p->ptr = Memory::realloc(p->ptr, p->size);
            }

            /**
             * Increase the size of the buffer.
             *
             * @param size Number of bytes to add after the existing ones.
             */
            inline void  grow    (int size) {
                VoidBuffer::realloc(VoidBuffer::size() + size);
            }
            /**
             * Decrease the size of the buffer.
             *
             * @param size Number of bytes to remove at the end of the buffer.
             */
            inline void  reduce  (int size) {
                VoidBuffer::realloc(VoidBuffer::size() - size);}

            /**
             * Cast to void *
             */
            inline operator void* () const {return ptr();}

            /**
             * Cast to something else
             */
            template <typename T>
            inline operator T*    () const {return (T*)ptr();}

            /**
             * Change the start address of the buffer.
             */
            inline const VoidBuffer &operator -= (int offset) {
                this->offset-=offset;
                return *this;
            }
            inline const VoidBuffer operator -  (int offset) const {
                VoidBuffer buf(*this);
                buf -= offset;
                return buf;
            }

            /**
             * Change the start address of the buffer.
             */
            inline const VoidBuffer &operator += (int offset) {
                this->offset+=offset;
                return *this;
            }
            inline const VoidBuffer operator + (int offset) const {
                VoidBuffer buf(*this);
                buf += offset;
                return buf;
            }

            inline int getOffset() const { return offset; }

            inline VoidBuffer dup() const {
                VoidBuffer buf(size());
                Memory::memcpy(buf.ptr(), ptr(), size());
                return buf;
            }

            inline void concat(const VoidBuffer &buf) const {
                int dsize = size();
                int ssize = buf.size();
                grow(buf.size());
                Memory::memcpy(ptr() + dsize, buf.ptr(), ssize());
            }

        private:
            Ptr *p;
            int offset;

            inline VoidBuffer (VoidBuffer &buf, int offset) {
              printf ("DO NOT USE THIS (%s::%d)\n", __FILE__, __LINE__);
            }

            inline VoidBuffer (const VoidBuffer &buf, int offset) {
              printf ("DO NOT USE THIS (%s::%d)\n", __FILE__, __LINE__);
            }
    };

	// DO NOT STORE OBJECT IN BUFFER, just pointers or primitive types.

    /// Simulate a (T*) buffer using VoidBuffer.
    template <typename T> class Buffer : public VoidBuffer {

        public:
            inline Buffer (int size = 0)          : VoidBuffer(size * sizeof(T)) {}
            inline Buffer (const T *data,int len) : VoidBuffer(data,len*sizeof(T)) {}
            inline Buffer (const VoidBuffer &buf) : VoidBuffer(buf) {}
            inline Buffer (      VoidBuffer &buf) : VoidBuffer(buf) {}
            inline Buffer (const Buffer<T>&buf)   : VoidBuffer(buf) {}
            inline Buffer (      Buffer<T>&buf)   : VoidBuffer(buf) {}

            inline const Buffer<T> operator=(const Buffer<T>&buf) {
              VoidBuffer::operator=(buf);
              return *this;
            }

            inline const Buffer<T> operator=(Buffer<T>&buf) {
              VoidBuffer::operator=(buf);
              return *this;
            }

            inline ~Buffer () {}

            inline void realloc  (int size) { VoidBuffer::realloc(size*sizeof(T));}
            inline void  grow    (int size) { VoidBuffer::grow   (size*sizeof(T));}
            inline void  reduce  (int size) { VoidBuffer::reduce (size*sizeof(T));}

            inline Buffer<T> dup() const {
                Buffer<T> buf(size());
                Memory::memcpy(buf.ptr(), ptr(), size());
                return buf;
            }
            
            inline T * ptr()  const {return (T*)VoidBuffer::ptr();}
            inline int size() const {return VoidBuffer::size() / sizeof(T);}

            inline operator T* () const {return ptr();}
            inline const Buffer<T>& operator -= (int offset) {
				VoidBuffer::operator-=(offset*sizeof(T));
				return *this;
            }
            inline const Buffer<T>  operator -  (int offset) const {
                Buffer<T> buf(*this);
                buf -= offset;
                return buf;
            }

            inline const Buffer<T>& operator += (int offset) {
				VoidBuffer::operator+=(offset*sizeof(T));
				return *this;
            }
            inline const Buffer<T>  operator +  (int offset) const {
                Buffer<T> buf(*this);
                buf += offset;
                return buf;
            }

#ifdef DEBUG_MEMORY
            inline T operator[] (int i) const;
            inline T &operator[] (int i);
#else
            inline T  operator[] (int i) const {return ptr()[i];}
            inline T &operator[] (int i)       {return ptr()[i];}
#endif
			inline T  get(int i) const          {return this->operator[](i);}
			inline T &get(int i)                {return this->operator[](i);}

        private:
            /* deprecated */
            inline Buffer (      Buffer<T>&buf,int offset) {
              printf ("DO NOT USE THIS (%s::%d)\n", __FILE__, __LINE__);
            }
            inline Buffer (const Buffer<T>&buf,int offset) {
              *(char*)0=0;
              printf ("DO NOT USE THIS (%s::%d)\n", __FILE__, __LINE__);
            }
    };

	// DO NOT STORE OBJECT IN BUFFER, just pointers or primitive types.

    template <typename T> class AdvancedBuffer : public Buffer<T> {

        public:
            inline AdvancedBuffer(int granularity = 64) : Buffer<T>(granularity),granularity(granularity),used(0) {}
            inline AdvancedBuffer(      AdvancedBuffer<T>&buf, int offset=0)
                : Buffer<T>(buf,offset), granularity(buf.granularity), used(buf.used - offset) {}
            inline AdvancedBuffer(const AdvancedBuffer<T>&buf)
                : Buffer<T>(buf), granularity(buf.granularity), used(buf.used) {}
			
			inline const AdvancedBuffer<T> operator=(      AdvancedBuffer<T>&buf) {
				Buffer<T>::operator=(buf);
				granularity = buf.granularity;
				used = buf.used;
				return *this;
			}

			inline const AdvancedBuffer<T> operator=(const AdvancedBuffer<T>&buf) {
				Buffer<T>::operator=(buf);
				granularity = buf.granularity;
				used = buf.used;
				return *this;
			}

            inline void add(const T element) {
                if(used>=capacity()) this->grow(this->granularity);
                (*this)[used++] = element;
            }
            inline void add() {
                if(used>=capacity()) this->grow(this->granularity);
                used++;
            }

            inline int  remove(const T t) {
                for (int index=size()-1;index>=0;--index)
                    if (this->get(index) == t) {
                        this->get(index) = this->get(size()-1);
                        used--;
                        return index;
                    }
                return -1;
            }
            inline void remove() { used--; }
            
            inline int capacity() const { return Buffer<T>::size(); }
            inline int size()     const { return used; }
            
            inline T &last()       { return this->ptr()[used-1]; }
            inline T  last() const { return this->ptr()[used-1]; }

            inline void clear()    { used = 0; }

            inline void flush()    { this->realloc(used); }
            inline bool isFlushed() const { return used == capacity(); }

            inline const AdvancedBuffer<T>  operator +  (int offset) const { return AdvancedBuffer<T>(*this,offset); }
            inline const AdvancedBuffer<T>& operator += (int offset) {
				used -= offset;
				Buffer<T>::operator+=(offset);
				return *this;
			}

        private:
            int granularity;
            int used;
    };

    template <typename T>
    class Stack {
        public:
            Stack(int granularity) : buffer(granularity) {}
            void push(T t) {buffer.add(t);}
            const T pop()  { T t = buffer.last(); buffer.remove(); return t; }
            const T top() const { return buffer.last(); }
            T &top()       { return buffer.last(); }

        private:
            AdvancedBuffer<T> buffer;
    };

    class String {
        public:
            String() : buffer(1) { buffer[0]=0; }
            String(const char *str) : buffer(str, strlen(str)+1) {}
            String(const String &s) : buffer(s.buffer.dup())    {} 
            
            void operator=(const String &s) { buffer = s.buffer; }
            ~String() {}

            String operator+(const String &s) const {
                String ret = this->dup();
                // buffer.size() + s.buffer.size() - 1);
                // strcpy(ret.buffer, buffer);
                // strcat(ret.buffer, s.buffer);
                ret += s;
                return ret;
            }

            String dup() const {
                return String (buffer.dup());
            }

            // WARNING USE ONLY FOR PRIMITIVE TYPE (int, float, ...)
            template<typename T>
            String concat(const char *format, const T t) const {
                static char txt[64];
                sprintf(txt, format, t);
                String ret(txt);
                return *this + ret;
            }

            String operator+(int i)    const { return concat<int>("%d", i); }
            String operator+(float f)  const { return concat<float>("%f", f); }
            String operator+(double d) const { return concat<double>("%g", d); }

            bool operator==(const String &s) const {
                return !strcmp(buffer, s.buffer);
            }

            const String& operator+=(const String &s) {
                buffer.grow(s.size()+1);
                strcat(buffer, s.buffer);
                return *this;
            }

            template<typename T>
            void append(const char *format, const T t) {
                char txt[128];
                sprintf(txt, format, t);
                buffer.grow(strlen(txt));
                strcat(buffer, txt);
            }

            const String& operator+=(int i)    { append<int>   ("%d", i); return *this; }
            const String& operator+=(float f)  { append<float> ("%f", f); return *this; }
            const String& operator+=(double d) { append<double>("%d", d); return *this; }

            void add(char c)      {
                if (c == '\0') fprintf(stderr, "WARNING: YOU'RE NOT ALLOWED TO ADD \\0 INTO A STRING!");
                int size = this->size();
                buffer[size] = c;
                buffer.grow(1);
                buffer[size+1] = 0;
            }
            void removeLastChar() { buffer[size()-1]=0; buffer.reduce(1); }

            /*int   size() const { return buffer.size() - 1; }
            int length() const { return buffer.size() - 1; }*/
            int   size() const { return strlen(buffer); }
            int length() const { return strlen(buffer); }

            const String substring(int i) const {
                String ret;
                ret.buffer = this->buffer + i;
                return ret;
            }

            String substring(int first, int last) const {
#ifdef DEBUG_MEMORY
                if (last < first) fprintf(stderr, "BAD SUBSTRING. %d >= %d\n", first, last);
                if (last > size()) fprintf(stderr, "BAD SUBSTRING %d > size(%d)\n", last, size());
#endif
                String ret = substring(first).dup();
                ret.buffer.realloc(last-first+1);
                ret.buffer[ret.buffer.size() - 1] = 0;
                return ret;
            }
            
            operator const char *() const { return (const char*)buffer; }
            char  operator[] (int i) const { return buffer[i]; }
            char &operator[] (int i)       { return buffer[i]; }
            
            inline int getOffset() const { return buffer.getOffset(); }
            
        protected:
            Buffer<char> buffer;

        private:
            String(int size) : buffer(size) {}
            String(Buffer<char> buf) : buffer(buf) {}
    };
};

#include "ios_exception.h"

#ifdef DEBUG_MEMORY

#define NB_MALLOC_MAX 0x80000

// memory allocation tracing functions.
int  findMemoryBlock(const void *ptr);
void addMemoryBlock(void *ptr, int size);
void removeMemoryBlock(void *ptr);

void *ios_fc::Memory::malloc (int size) {

    if (size<0)
        IOS_ERROR("Bad Malloc");
    if (size == 0) size = 1;
    void *ret = ::malloc(size);
    if (ret == 0) IOS_ERROR("Malloc failed, not enough memory?");

    addMemoryBlock(ret, size);

    return ret;
}

void *ios_fc::Memory::calloc (int size) {
    
    if (size < 0) IOS_ERROR ("Bad Calloc");
    if (size == 0) size = 1;
    void *ret = ::calloc(1,size);
    if (ret == 0) IOS_ERROR ("Calloc failed, not enough memory?");

    addMemoryBlock(ret, size);
    
    return ret;
}

void *ios_fc::Memory::realloc (void *ptr, int size) {

    if (size<0) IOS_ERROR ("Bad realloc");
    if (size == 0) size = 1;

    removeMemoryBlock(ptr);
//    fprintf(stderr, "REALLOC %x\n",ptr);
    
    void *ret = ::realloc(ptr,size);
    if (ret == 0) IOS_ERROR ("Realloc failed, not enough memory?");

    addMemoryBlock(ret, size);
    
    return ret;
}

void ios_fc::Memory::memcpy (void *dest, const void *src, int len) {
    
    if (len<0) IOS_ERROR ("Bad memcpy");

    int idst = findMemoryBlock(dest);
//    int isrc = findMemoryBlock(src);

    if (idst < 0) fprintf(stderr, "SPURIOUS memcpy destination");
//    if (isrc < 0) fprintf(stderr, "SPURIOUS memcpy src");
    
    ::memcpy(dest,src,len);
}

void ios_fc::Memory::free (void *ptr) {

    removeMemoryBlock(ptr);
    
    ::free(ptr);
}
            
template<typename T>
    inline T ios_fc::Buffer<T>::operator[] (int i) const {
        if ((i>=0)&&(i<size()))
            return ptr()[i];
        else
            IOS_ERROR("Index out of bounds");
    }

template<typename T>
    inline T &ios_fc::Buffer<T>::operator[] (int i) {
        if ((i>=0)&&(i<size()))
            return ptr()[i];
        else
            IOS_ERROR("Index out of bounds");
    }

inline void ios_fc::VoidBuffer::init_ptr (int size) {

    p       = new Ptr();
    if (size < 0) IOS_ERROR ("Bad buffer size.");
    if (size)
        p->ptr  = Memory::malloc(size);
    else
        p->ptr  = Memory::malloc(1);
    p->size = size;
    p->nb   = 1;
}
#endif

#endif
