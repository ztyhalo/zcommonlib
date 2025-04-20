#ifndef ZSYSTEMSEM_H
#define ZSYSTEMSEM_H

#include <string>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "zprint.h"

using namespace std;

#define EINTR_LOOP(var, cmd)                    \
do {                                        \
        var = cmd;                              \
} while (var == -1 && errno == EINTR)

class ZSystemSem
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

public:
    key_t  m_unix_key;
    int    m_sem;
    int    m_initVal;
    bool   m_createdFile;
    bool   m_createdSem;

    string m_key;
    string m_fileName;


public:
    ZSystemSem();
    virtual ~ZSystemSem()
    {
        zprintf3("ZSystemSem destruct!\n");
        cleanHandle();
    }

    inline string makeKeyFileName() const;

    key_t handle(AccessMode mode = Open);
    void cleanHandle();
    bool modifySemaphore(int count);
    SemOpTimeState modifySemaphore(int count, int ms);
    int  createUnixKeyFile(const string &fileName);
    void setKey(const string & key, int initVal = 0, AccessMode mode = Open);
    string key() const;
    bool acquire();
    bool release(int n = 1);
    bool syssemOk() const
    {
        return (m_unix_key != -1);
    }
};

#endif // ZSYSTEMSEM_H
