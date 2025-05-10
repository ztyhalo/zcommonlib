#include "zsysshmbase.h"
#include "zprint.h"

ZSysShmBase::ZSysShmBase():m_memory(0),m_size(0),m_state(SHMOK),m_createShm(0)
{
    ;
}

ZSysShmBase::~ZSysShmBase()
{
    zprintf3("ZSysShmBase destruct\n");
}

bool ZSysShmBase::lock()
{
    m_shmMutex.lock();
    return semLock();
}

bool ZSysShmBase::unlock()
{
    semUnlock();
    return m_shmMutex.unlock();
}
