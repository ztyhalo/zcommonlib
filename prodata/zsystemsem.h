#ifndef ZSYSTEMSEM_H
#define ZSYSTEMSEM_H

#include <string>
#include <sys/sem.h>
#include <sys/ipc.h>
#include "zprint.h"

using namespace std;

class ZSystemSem
{
    enum AccessMode
    {
        Open,
        Create
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
    }

    inline string makeKeyFileName() const;

    key_t handle(AccessMode mode = Open);
    void cleanHandle();
    bool modifySemaphore(int count);
    int  createUnixKeyFile(const string &fileName);
    void setKey(const string & key, int initVal = 0, AccessMode mode = Open);
    string key() const;
    bool acquire();
    bool release(int n = 1);
};

#endif // ZSYSTEMSEM_H
