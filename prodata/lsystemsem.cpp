#include "lsystemsem.h"

LSystemSem::LSystemSem():m_semKey(-1),m_semId(-1),m_initVal(0),m_created(false)
{
    ;
}

LSystemSem::~LSystemSem()
{
    zprintf3("LSystemSem destruct!\n");
    cleanHandle();
}

void LSystemSem::cleanHandle()
{
    if(m_created)
    {
        if(-1 != m_semId)
        {
            if(-1 == semctl(m_semId, 0, IPC_RMID, 0))
            {
                zprintf1("LSystemSem rm msem %d error!\n", m_semId);
            }
            m_semId = -1;
        }
        m_created = false;
    }

}


int LSystemSem::handle(AccessMode mode)
{
    if(-1 != m_semId)
        return m_semId;
    if(m_semKey <= 0)
    {
        zprintf1("LSystemSem m_semKey empty!\n");
        return -1;
    }

    m_semId = semget(m_semKey, 1, 0600 | IPC_CREAT | IPC_EXCL);
    if(-1 == m_semId)
    {
        if(errno == EEXIST)
            m_semId = semget(m_semKey, 1, 0600 | IPC_CREAT);
        if(-1 == m_semId)
        {
            zprintf1("semget m_semKey %d error!\n", m_semKey);
            cleanHandle();    //缺少清理函数
            return -1;
        }
        if(mode == Create)
        {
            m_created = true;
        }
    }
    else
    {
        m_created = true;
    }

    if (m_created && m_initVal >= 0) {
        semun init_op;
        init_op.val = m_initVal;
        if (-1 == semctl(m_semId, 0, SETVAL, init_op)) {
            zprintf1("init sem %d error!\n", m_semKey);
            perror("Sem init");
            cleanHandle();
            return -1;
        }
    }

    return 0;
}


int LSystemSem::readSemKey(key_t semkey)
{
     m_semId = semget(semkey, 0, 0600);
    if(-1 == m_semId)
    {
        if (errno == ENOENT)
        {
            return -1;             //不存在该信号量
        }
        else
        {
            perror("Sem Get Error!\n");
            return -2;
        }
    }
    m_semKey = semkey;
    return 0;
}
int LSystemSem::createSem(key_t semkey , int initval)
{
    m_semId = semget(semkey, 1, 0600 | IPC_CREAT | IPC_EXCL);  //不存在则创建，存在返回错误eexist
    if(-1 == m_semId)
    {
        if(errno == EEXIST)
        {
            zprintf1("LSystemSem createsem %d is exist!\n", semkey);
            m_created = false;
            m_semId = semget(semkey, 1, 0600 | IPC_CREAT);
        }
        if(-1 == m_semId)
        {
            zprintf1("semget key %d error!\n", semkey);
            return -1;
        }
    }
    else
    {
        m_created = true;
    }

    if (m_created && initval >= 0)
    {
        semun init_op;
        init_op.val = initval;
        if (-1 == semctl(m_semId, 0, SETVAL, init_op))
        {
            zprintf1("init sem %d error!\n", m_semId);
            perror("new Sem init");
            return -1;
        }
        m_initVal = initval;
    }
    return 0;
}
int LSystemSem::setKey(key_t semkey , int initval, AccessMode mode)
{
    if(mode == Open)
    {
        return readSemKey(semkey);
    }
    else
        return createSem(semkey, initval);
}

bool LSystemSem::modifySemaphore(int count)
{
    if(-1 == handle())
    {
        return false;
    }

    struct sembuf operation;
    operation.sem_num = 0;
    operation.sem_op = count;
    operation.sem_flg = SEM_UNDO;

    int res;
    res = semop(m_semId, &operation, 1);
    if( -1 == res)
    {
        if(errno == EINVAL || errno == EIDRM)
        {
            m_semId = -1;
            cleanHandle();
            handle();
            return modifySemaphore(count);
        }
        zprintf1("semop m_semId %d error!\n", m_semId);
        return false;
    }
    return true;
}

LSystemSem::SemOpTimeState LSystemSem::modifySemaphore(int count, int ms)
{
    if(-1 == handle())
    {
        return PError;
    }

    struct sembuf operation;
    operation.sem_num = 0;
    operation.sem_op = count;
    operation.sem_flg = SEM_UNDO;

    struct timespec timeout;
    timeout.tv_sec  = (ms / 1000);
    timeout.tv_nsec = (ms - timeout.tv_sec * 1000L) * 1000000L;

    // register int res;
    int res;
    res = semtimedop(m_semId, &operation, 1, &timeout);
    if( -1 == res)
    {
        if(errno == EINVAL || errno == EIDRM)
        {
            m_semId = -1;
            cleanHandle();
            handle();
            return modifySemaphore(count, ms);
        }
        else if(errno == ETIMEDOUT)
            return PTimeOver;
        zprintf1("semop m_semId %d error!\n", m_semId);
        return PError;
    }
    return Pok;
}


bool LSystemSem::acquire()
{
    return modifySemaphore(-1);
}

bool LSystemSem::release(int n)
{
    if(n == 0)
        return true;
    if(n < 0)
    {
        zprintf1("system sem realease n is negative!\n");
        return false;
    }
    return modifySemaphore(n);
}

