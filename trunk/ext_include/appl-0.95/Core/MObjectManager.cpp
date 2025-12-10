#include "MObjectManager.h"

using namespace momdp;

template <typename T>
MObjectManager<T>::MObjectManager(void)
{
}

template <typename T>
MObjectManager<T>::~MObjectManager(void)
{
}

// delete the MObject from everywhere
template <typename T>
void MObjectManager<T>::strongDelete(T *pointer)
{
    for(typename list<MObjectUser<T> *>::iterator iter = userList.begin(); iter != userList.end(); iter ++)
    {
        (*iter)->forcedDelete(pointer);
    }
    localDelete(pointer);
}


// one user deleted the MObject, but the MObject may still be used by other users. Only decrease refCount
template <typename T>
void MObjectManager<T>::weakDelete(T *pointer)
{
    // TODO: obsolete this by using smart pointers
    //pointer->refCount -- ;
    //if(pointer->refCount == 1)
    //{
    //	// only the manager is using it, may do something here...
    //}
    //else if(pointer->refCount == 0)
    //{
    //	// no one is using it, delete
    //	localDelete(pointer);
    //}


}


template <typename T>
void MObjectManager<T>::registerUser(MObjectUser<T> *pointer)
{
    userList.push_back(pointer);
}

template <typename T>
void MObjectManager<T>::removeUser(MObjectUser<T> *pointer)
{
    userList.remove(pointer);
}
