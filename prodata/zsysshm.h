#ifndef ZSYSSHM_H
#define ZSYSSHM_H
#include "zsystemsem.h"
// #include "mutex_class.h"
#include "zsysshmbase.h"






class ZSysShm:public ZSysShmBase
{
  public:

    key_t       m_shmKey;
    int         m_shmId;
    ZSystemSem  m_sysSem;

  public:
    ZSysShm();
    virtual ~ZSysShm();

    bool init_key();
    void setKey(key_t key);
    bool semLock() override;
    bool semUnlock() override;

    bool create(int size);
    bool attach(AccessMode mode = Create);
    bool isAttached() const;
    bool detach();
    int clean_handle();
    int createData(int size);
    int createData(int size, key_t id);
    int readCreateData(key_t id, int size = 0);
    int size() const override;
    void *data() override;
};

#endif // ZSYSSHM_H
