#ifndef LSYSTEMSEM_H
#define LSYSTEMSEM_H

#include <sys/sem.h>
#include <sys/ipc.h>
#include "zprint.h"

using namespace std;

#define EINTR_LOOP(var, cmd)                    \
do {                                        \
        var = cmd;                              \
} while (var == -1 && errno == EINTR)

class LSystemSem
{
public:
    enum AccessMode
    {
        Open,
        Create
    };

    enum SemOpTimeState
    {
        PError,
        Pok,
        PTimeOver
    };

    union semun
    {
        int              val;
        struct semid_ds* buf;
        unsigned short*  array;
    };

public:
    key_t  m_semKey;
    int    m_semId;
    int    m_initVal;
    bool   m_created;

public:
    LSystemSem();
    virtual ~LSystemSem()
    {
        zprintf3("LSystemSem destruct!\n");
        cleanHandle();
    }

    int handle(AccessMode mode = Open);
    void cleanHandle();
    bool modifySemaphore(int count);
    SemOpTimeState modifySemaphore(int count, int ms);
    int setKey(key_t semkey , int initVal = 0, AccessMode mode = Open);
    int createSem(key_t semkey , int initVal = 0);
    int readSemKey(key_t semkey);
    bool acquire();
    bool release(int n = 1);
    bool syssemOk() const
    {
        return (m_semId != -1);
    }
    int getSemCount() const
    {
        if(m_semId == -1)
            return -1;
        union semun sem_union;

        return semctl(m_semId, 0, GETVAL, sem_union);

    }

};



#endif // LSYSTEMSEM_H
