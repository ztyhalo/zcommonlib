/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2017.7.11 将1个文件拆分为多个文件
 *         2025.4.18 优化
 *
 ****************************************************/

#ifndef __SHAREMEM_H__
#define __SHAREMEM_H__

#include <QVector>
#include <QString>
#include <QSharedMemory>
#include "pro_data.h"
#include <string.h>
#include "zsystemsem.h"
#include "zlockerclass.h"

using namespace std;


// linux共享内存 不继承线程类
template < class T >
class ShareDataT : public creatdata< T >
{
public:
   enum AccessMode
    {
        ReadOnly,
        ReadWrite
    };
  public:

    key_t       m_shmKey;
    void *      m_memory;
    ZSystemSem  m_sysSem;

  public:
    ShareDataT():m_shmKey(0),m_memory(0)
    {
        ;
    }
    virtual ~ShareDataT()
    {
        zprintf3("share data destruct\n");
        if(isAttached())
        {
            detach();
        }
        this->m_data = NULL;
    }
    bool init_key();
    bool lock();
    bool unlock();
    bool create(int size);
    bool attach(AccessMode mode = ReadWrite);
    bool isAttached() const;
    bool detach();
    int clean_handle();
    int creat_data(int size) override;
    int creat_data(int size, key_t id);
    int read_creat_data(key_t id = 20130408, int size = 0);
};

template < class T >
int ShareDataT< T >::clean_handle()
{
    m_shmKey = 0;
    return 0;
}

template < class T >
bool ShareDataT< T >::isAttached() const
{
    return (0 != m_memory);
}

template < class T >
bool ShareDataT< T >::lock()
{
    if(m_sysSem.acquire())
    {
        return true;
    }
    zprintf1("share data t lock error!\n");
    return false;
}

template < class T >
bool ShareDataT< T >::unlock()
{
    if(m_sysSem.release())
    {
        return true;
    }
    zprintf1("share data t unlock error!\n");
    return false;
}

template < class T >
bool ShareDataT< T >::create(int size)
{
    bool ret = true;
    if(-1 == shmget(m_shmKey, size, 0600 | IPC_CREAT |IPC_EXCL)) //不存在则创建，存在返回错误eexist
    {
        switch(errno)
        {
            case EINVAL:
                zprintf1("share data t create fail!\n");
                ret = false;
                break;
            case EEXIST:
                zprintf2("create shm id %d is have!\n", m_shmKey);
                break;
            default:
                zprintf1("create shm id errno %d!\n", errno);
                ret = false;
        }
    }
    return ret;
}
template < class T >
bool ShareDataT< T >::attach(AccessMode mode)
{
    int   shmid = shmget(m_shmKey, 0, (mode == ReadOnly ? 0400 : 0600));
    int   size;
    if(-1 == shmid)
    {
        zprintf1("ShareDataT m_shmKey %d attach error!\n", m_shmKey);
        return false;
    }

    m_memory = shmat(shmid, 0, (mode == ReadOnly ? SHM_RDONLY : 0));
    if ((void*) - 1 == m_memory) {
        m_memory = 0;
        zprintf1("share data ::attach (shmat) error!\n");
        return false;
    }

    shmid_ds shmid_ds;
    if (!shmctl(shmid, IPC_STAT, &shmid_ds)) {
        size = (int)shmid_ds.shm_segsz;
    } else {
        zprintf1("share data::attach (shmctl) error!\n");
        return false;
    }

    if(size != this->m_size)
    {
        zprintf1("share data: size %d m_size %d!\n", size, this->m_size);
        this->m_size = size;
    }
    return true;
}

template < class T >
bool ShareDataT< T >::detach()
{
    ZLockerClass<ShareDataT<T>> locker(this);
    locker.lock();
    if(-1 == shmdt(m_memory))
    {
        switch(errno)
        {
            case EINVAL:
                zprintf1("ShareDataT detach no attached!\n");
                break;
            default:
                zprintf1("ShareDataT detach errno %d!\n", errno);
        }
        return false;
    }

    m_memory = 0;
    this->m_size = 0;

    int id = shmget(m_shmKey, 0, 0400);
    clean_handle();

    struct shmid_ds shmid_ds;
    if (0 != shmctl(id, IPC_STAT, &shmid_ds)) {
        switch (errno) {
        case EINVAL:
            return true;
        default:
            return false;
        }
    }

    if(shmid_ds.shm_nattch == 0)
    {
        if(-1 == shmctl(id, IPC_RMID, &shmid_ds))
        {
            zprintf1("ShareDataT detach IPC_RMID err!\n");
            switch (errno) {
            case EINVAL:
                return true;
            default:
                return false;
            }
        }
    }

    return true;
}

template < class T >
bool ShareDataT< T >::init_key()
{
    m_sysSem.setKey(string(), 1);
    m_sysSem.setKey(std::to_string(m_shmKey), 1);
    return m_sysSem.syssemOk();
}

template < class T >
int ShareDataT< T >::creat_data(int size)
{
    zprintf2("shm id is %d\n", m_shmKey);

    if(!init_key())
    {
        zprintf1("sharedatat creat data init key err!\n");
        return -1;
    }
    m_sysSem.setKey(std::to_string(m_shmKey), 1, ZSystemSem::Create);

    ZLockerClass<ShareDataT<T>> locker(this);
    locker.lock();

    if(!create(size))
        return -2;
    this->m_size = size;

    if(!attach())
    {
        zprintf1("ShareDataT attach fail!\n");
        return -3;
    }

    this->m_data = (T*) m_memory;

    return 0;
}

template < class T >
int ShareDataT< T >::creat_data(int size, key_t id)
{
    m_shmKey = id;
    return creat_data(size);
}

template < class T >
int ShareDataT< T >::read_creat_data(key_t id, int size)
{

    ZLockerClass<ShareDataT<T>> locker(this);
    locker.lock();

    if(0 != m_shmKey)
    {
        zprintf1("ShareDataT read_creat_data m_shm_key %d is have!\n", m_shmKey);
        return -1;
    }

    m_shmKey = id;
    this->m_size = size;
    if(!attach())
    {
        zprintf1("ShareDataT read_creat_data attach err!\n");
        return -2;
    }

    this->data = (T*) m_memory;
    return 0;
}

#endif /*__SHAREMEM_H__*/
