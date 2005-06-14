#ifndef _PTR_H
#define _PTR_H

namespace ios_fc {

// TODO: throw exceptions
	
template <typename X>
class SharedPtr
{
	private:
		struct Ref {
			X   *px;
			int  refCount;

			Ref(X *px) : px(px), refCount(1) {}
			~Ref() { if (px) delete px; }
			
			void inc() { refCount++; }
			void dec() { refCount--; }
		};
		Ref *ref;

		void dec() {
			ref->dec();
			if (ref->refCount == 0) delete ref;
		}

	public:
		SharedPtr(X* px = NULL)  { ref = new Ref(px); }
		SharedPtr(const SharedPtr<X> &ptr) : ref(ptr.ref) { ref->inc(); }
		~SharedPtr()                                { dec(); }
		
		SharedPtr &operator= (const SharedPtr<X> &ptr) {
			dec();
			ref = ptr.ref;
			ref->inc();
                        return *this;
		} 

		void release()        { ref->px = NULL;     }
		X *get()        const { return ref->px;  }
		X &operator*()  const { return *ref->px; }
		X *operator->() const { return ref->px;  }
		bool operator!() const { return (ref->px == NULL); }
};

}

#define Ptr SharedPtr

#endif

