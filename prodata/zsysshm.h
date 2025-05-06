#ifndef ZSYSSHM_H
#define ZSYSSHM_H
#include "zsystemsem.h"
#include "mutex_class.h"

class ZSysShm
{
  public:
    enum AccessMode
    {
        ReadOnly,
        ReadWrite
    };
    enum ShmState
    {
        SHMOK,
        SHMEXIST,
        SHMSIZEERR
    };

  public:

    key_t       m_shmKey;
    void *      m_memory;
    int         m_size;
    ShmState    m_state;
    int         m_createShm;
    ZSystemSem  m_sysSem;
    MUTEX_CLASS m_shmMutex;

  public:
    ZSysShm();
    virtual ~ZSysShm();

    bool init_key();
    bool semLock();
    bool semUnlock();
    bool lock();
    bool unlock();
    bool create(int size);
    bool attach(AccessMode mode = ReadWrite);
    bool isAttached() const;
    bool detach();
    int clean_handle();
    int createData(int size);
    int createData(int size, key_t id);
    int readCreateData(key_t id, int size = 0);
    int size() const;
    void *data();
};

#endif // ZSYSSHM_H
