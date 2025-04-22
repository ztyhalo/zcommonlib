#ifndef ZLOCKERCLASS_H
#define ZLOCKERCLASS_H

#include "zprint.h"

using namespace std;

template < class T >
class ZLockerClass
{

public:
    inline explicit ZLockerClass(T *lockClass) : q_sm(lockClass)
    {
        if(q_sm == NULL)
        {
            zprintf1("zlock class error!\n");
        }
    }

    inline ~ZLockerClass()
    {
        if (q_sm)
            q_sm->unlock();
    }

    inline bool lock()
    {
        if (q_sm && q_sm->lock())
            return true;
        q_sm = 0;
        return false;
    }

private:
    T *q_sm;
};

#define ZLockerMutex ZLockerClass<MUTEX_CLASS>
#endif // ZLOCKERCLASS_H
