#ifndef ZSHAREMEMRW_H
#define ZSHAREMEMRW_H
#include "lsystemsem.h"

class ZSharememRW
{
public:

    int         m_bufSize;
    int     *   m_pRd;
    int     *   m_pWr;
    int     *   m_pSize;

    LSystemSem  m_sem;

public:
    ZSharememRW():m_bufSize(0),
        m_pRd(NULL),m_pWr(NULL),m_pSize(NULL)
    {
        ;
    }
    virtual ~ZSharememRW()
    {
        zprintf3("ZSharememRW destruct!\n");
        cleanHandle();
    }

    void cleanHandle()
    {
        m_sem.cleanHandle();
    }
    void realeseSem()
    {
        m_sem.release(1);
    }

    int wait_thread_sem(void)
    {
        if(m_sem.acquire() ==true)
            return 0;
        else
            return -1;
    }
};





#endif // ZSHAREMEMRW_H
