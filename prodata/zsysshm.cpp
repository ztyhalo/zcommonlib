#include "zsysshm.h"
#include <sys/shm.h>
#include "zlockerclass.h"

ZSysShm::ZSysShm():m_shmKey(0),m_memory(0),m_size(0),m_state(SHMOK),m_createShm(0)
{
    ;
}

ZSysShm::~ZSysShm()
{
    zprintf3("share data destruct\n");
    if(isAttached())
    {
        detach();
    }
    // this->m_data = NULL;
}


bool ZSysShm::init_key()
{
    m_sysSem.setKey(string(), 1);
    m_sysSem.setKey(std::to_string(m_shmKey), 1);
    return m_sysSem.syssemOk();
}


int ZSysShm::clean_handle()
{
    m_shmKey = 0;
    return 0;
}

bool ZSysShm::isAttached() const
{
    return (0 != m_memory);
}


bool ZSysShm::semLock()
{
    if(m_sysSem.acquire())
    {
        return true;
    }
    zprintf1("share data t lock error!\n");
    return false;
}

bool ZSysShm::semUnlock()
{
    if(m_sysSem.release())
    {
        return true;
    }
    zprintf1("share data t unlock error!\n");
    return false;
}

bool ZSysShm::lock()
{
    m_shmMutex.lock();
    return semLock();
}

bool ZSysShm::unlock()
{
    semUnlock();
    return m_shmMutex.unlock();
}

bool ZSysShm::create(int size)
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
                m_state = SHMEXIST;
                ret = false;
                break;
            default:
                zprintf1("create shm id errno %d!\n", errno);
                ret = false;
        }
    }
    m_createShm = 1;
    return ret;
}

bool ZSysShm::attach(AccessMode mode)
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
        m_state = SHMSIZEERR;
        if(!detach())
            zprintf1("ZSysShm attach detach error!\n");
        return false;
    }
    return true;
}

bool ZSysShm::detach()
{
    ZLockerClass<ZSysShm> locker(this);
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
    m_size = 0;

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
        m_createShm = 0;
    }
    if(m_createShm)
    {
        if(shmid_ds.shm_nattch != 0)
        {
            zprintf1("warning ZSysShm release error!\n");
            shmid_ds.shm_nattch = 0;
        }
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
        m_createShm = 0;
    }

    return true;
}


int ZSysShm::createData(int size)
{
    zprintf2("shm id is %d\n", m_shmKey);

    if(!init_key())
    {
        zprintf1("sharedatat creat data init key err!\n");
        return -1;
    }
    m_sysSem.setKey(std::to_string(m_shmKey), 1, ZSystemSem::Create);

    ZLockerClass<ZSysShm> locker(this);
    locker.lock();

    if(!create(size))
        return -2;
    this->m_size = size;

    if(!attach())
    {
        zprintf1("ShareDataT attach fail!\n");
        return -3;
    }

    return 0;
}

int ZSysShm::createData(int size, key_t id)
{
    m_shmKey = id;
    return createData(size);
}

int ZSysShm::readCreateData(key_t id, int size)
{

    ZLockerClass<ZSysShm> locker(this);
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

    return 0;
}

void *ZSysShm::data()
{
    return m_memory;
}

int ZSysShm::size() const
{
    return m_size;
}
