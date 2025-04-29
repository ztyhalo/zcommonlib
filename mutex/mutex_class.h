#ifndef __MUTEX_CLASS_H__
#define __MUTEX_CLASS_H__

#include <pthread.h>



class MUTEX_CLASS
{
private:
    pthread_mutex_t   mut;
public:
    MUTEX_CLASS();
    virtual ~MUTEX_CLASS();
    bool lock(void);
    bool unlock(void);
};

#endif //__MUTEX_CLASS_H__
