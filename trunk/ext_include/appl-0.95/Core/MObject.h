#ifndef MObject_H
#define MObject_H

#include <cstddef>
#include <string>

//38	#include <boost/intrusive_ptr.hpp>
// using boost::intrusive_ptr;
// Gcc 4.7.0 and clang 3.1 need the following two lines to compile this example
//using boost::intrusive_ptr_add_ref;
//using boost::intrusive_ptr_release;

using namespace std;
namespace momdp
{
	class MObject;
}
using namespace momdp;

namespace boost
{
	void intrusive_ptr_add_ref(MObject * p);
	void intrusive_ptr_release(MObject * p);
};

namespace momdp 
{
	// Base class similar to Java Object
	// Provide following functions:
	// Memory usage counting:
	//		thisSize 
	//		update Memory usage to GlobalResource::getInstance()->memoryUsage by overloading new and delete 

	// Reference counting for intrusive smart point : referenceCount

	class MObject
	{
	private:
		int thisSize;
		int referenceCount;
		friend void ::boost::intrusive_ptr_add_ref(MObject * p);
		friend void ::boost::intrusive_ptr_release(MObject * p);

	public:
		MObject(void);
		virtual ~MObject(void);

		virtual string ToString();

		void*   operator    new(size_t nSize);
		void    operator    delete(void* p);

	};

}

namespace boost
{
	// WARNING: this implementation is not thread-safe
	// add in mutex for critical section to make it thread-safe
	// remember to modify and test this section when APPL become multi-threaded program...

    inline void intrusive_ptr_add_ref(MObject * p)
    {
        // increment reference count of object *p
        ++(p->referenceCount);
    }



    inline void intrusive_ptr_release(MObject * p)
    {
        // decrement reference count, and delete object when reference count reaches 0
        if (--(p->referenceCount) == 0)
        {
            delete p;
        }
    }
} // namespace boost

#include "boost/smart_ptr/intrusive_ptr.hpp"
#include "boost/smart_ptr/shared_ptr.hpp"

#define SharedPointer boost::intrusive_ptr
#define dynamic_pointer_cast boost::dynamic_pointer_cast
#define static_pointer_cast boost::static_pointer_cast

#endif

