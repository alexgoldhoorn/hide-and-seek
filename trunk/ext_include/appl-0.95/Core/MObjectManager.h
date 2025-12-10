#ifndef MObjectManager_H
#define MObjectManager_H

#include <list>
using namespace std;

#include "MObjectUser.h"


using namespace momdp;
namespace momdp 
{

class MObject;

template <typename T>
class MObjectManager
{
private:
	list<MObjectUser<T> *> userList;

public:

    MObjectManager(void);
    virtual ~MObjectManager(void);

	// delete the MObject from everywhere
    virtual void strongDelete(T *pointer);


	// one user deleted the MObject, but the MObject may still be used by other users. Only decrease refCount
    virtual void weakDelete(T *pointer);
	// this function perform the actual delete, must be instantiated
	virtual void localDelete(T *pointer) = 0; 


    virtual void registerUser(MObjectUser<T> *pointer);
    virtual void removeUser(MObjectUser<T> *pointer);

	
};
}

#endif

