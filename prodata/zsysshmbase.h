#ifndef ZSYSSHMBASE_H
#define ZSYSSHMBASE_H

// #include "zsystemsem.h"
#include "mutex_class.h"

class ZSysShmBase
{
public:
    enum AccessMode
    {
        Open,
        Create
    };
    enum ShmState
    {
        SHMOK,
        SHMEXIST,
        SHMSIZEERR
    };
    // enum ShmErr
    // {

    // };

public:

    void *      m_memory;
    int         m_size;
    ShmState    m_state;
    int         m_createShm;
    MUTEX_CLASS m_shmMutex;

public:
    ZSysShmBase();
    virtual ~ZSysShmBase();

    virtual bool semLock() = 0;
    virtual bool semUnlock() = 0;
    bool lock();
    bool unlock();

    virtual int size() const = 0;
    virtual void *data() = 0;
};

#endif // ZSYSSHMBASE_H
