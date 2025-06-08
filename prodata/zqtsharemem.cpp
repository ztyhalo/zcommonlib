#include "zqtsharemem.h"
#include <sys/ipc.h>
#include <sys/shm.h>

bool ZQTShareMem::destory(void)
{
    key_t unix_key;

    unix_key = ftok(m_lhshare.nativeKey().toStdString().c_str(), 'Q');
    if(unix_key == -1)
    {
        return true;
    }
    zprintf2("ZQTShareMem is exist key:%s nativekey: %s!\n", m_lhshare.key().toStdString().c_str(),
             m_lhshare.nativeKey().toStdString().c_str());
    int id = shmget(unix_key, 0, 0400);

    zprintf1("get unix_key %d id %d!\n", unix_key, id);
    if(id > 0)
    {
        struct shmid_ds shmid_ds;
        if (0 != shmctl(id, IPC_STAT, &shmid_ds))
        {
            switch (errno) {
            case EINVAL:
                return true;
            default:
                return false;
            }
        }
        if (shmid_ds.shm_nattch == 0)
        {
            // mark for removal
            if (-1 == shmctl(id, IPC_RMID, &shmid_ds)) {
                zprintf1("remove error %d!\n", errno);
                switch (errno) {
                case EINVAL:
                    m_createShm = 0;
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
                zprintf1("warning ZQTShareMem release error!\n");
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
    }
    // remove file
    if (!remove(m_lhshare.nativeKey().toStdString().c_str()))
        return false;

    return true;
}

int ZQTShareMem::newcreateData(int size)
{

    m_lhshare.setKey(m_shmKey);
    if (m_lhshare.isAttached())
    {
        zprintf2("qt share have attach!\n");
        m_lhshare.detach();
    }
    destory();

    if (!m_lhshare.create(size))
    {
        zprintf2("qt share %s creat fail!\n", m_shmKey.toStdString().c_str());

        if (m_lhshare.error() == QSharedMemory::AlreadyExists) //已经存在
        {
            if(!destory())
            {
                if (!m_lhshare.create(size))
                {
                    zprintf2("qt share %s creat fail err %d again!\n", m_shmKey.toStdString().c_str(), errno);
                    return -1;
                }
            }
            else
            {
                zprintf1("ZQTShareMem destory error!\n");
                return -2;
            }
        }
        else
        {
            zprintf1("ZQTShareMem creat data fail!\n");
            return -3;
        }
    }
    if(m_lhshare.isAttached())
    {
        zprintf2("qt create share have attach ok!\n");
    }
    else if (!m_lhshare.attach())
    {
        zprintf1("ZQTShareMem can't attatch qt share\n", m_shmKey.toStdString().c_str());
        return -4;
    }


    m_createShm = true;


    return 0;
}

int ZQTShareMem::readCreateData(int size, const QString & keyid)
{
    m_shmKey = keyid;

    m_lhshare.setKey(m_shmKey);
    zprintf3("ZQTShareMem read create %s!\n", keyid.toStdString().c_str());
    if (!m_lhshare.attach())
    {
        zprintf1("ZQTShareMem can't attatch qt share\n", keyid.toStdString().c_str());
        return -1;
    }


    if(size != m_lhshare.size())
    {
        zprintf1("ZQTShareMem read create size %d create size %d!\n", size, m_lhshare.size());
        return -2;
    }

    return 0;
}

int ZQTShareMem::readCreate(const QString & keyid)
{
    m_shmKey = keyid;

    m_lhshare.setKey(m_shmKey);

    if (!m_lhshare.attach())
    {
        zprintf1("ZQTShareMem can't attatch qt share\n", keyid.toStdString().c_str());
        return -1;
    }

    m_size =  m_lhshare.size();

    return 0;
}

void* ZQTShareMem::createData(int size, const QString & keyid, AccessMode mode)
{
    if(mode == Open)
    {
        zprintf1("ZQTShareMem read key %s!\n", keyid.toStdString().c_str());
        if(readCreateData(size, keyid) == 0)
        {
            zprintf3("ZQTShareMem read key %s ok!\n", keyid.toStdString().c_str());
            return m_lhshare.data();
        }
        else
        {
            zprintf1("ZQTShareMem read key %s error!\n", keyid.toStdString().c_str());
            return NULL;
        }
    }
    else
    {
        zprintf1("ZQTShareMem create key %s!\n", keyid.toStdString().c_str());
        m_shmKey = keyid;
        if (newcreateData(size) == 0)
        {
            zprintf3("ZQTShareMem create key %s ok!\n", keyid.toStdString().c_str());
            return m_lhshare.data();
        }
        else
        {
            zprintf1("ZQTShareMem create key %s error!\n", keyid.toStdString().c_str());
            return NULL;
        }
    }
}

int ZQTShareMem::size() const
{
    return m_lhshare.size();
}
bool ZQTShareMem::semLock()
{
    return m_lhshare.lock();
}

bool ZQTShareMem::semUnlock()
{
    return m_lhshare.unlock();
}
// bool ZQTShareMem::lock(void)
// {
//     m_mutex.lock();
//     return m_lhshare.lock();
// }
// bool ZQTShareMem::unlock(void)
// {
//     m_lhshare.unlock();
//     return m_mutex.unlock();
// }

void *ZQTShareMem::data()
{
    return m_lhshare.data();
}
