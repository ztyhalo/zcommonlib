#ifndef ZSYSTEMSEM_H
#define ZSYSTEMSEM_H

#include <string>
#include "lsystemsem.h"

using namespace std;


class ZSystemSem:public LSystemSem
{
// public:
//     enum AccessMode
//     {
//         Open,
//         Create
//     };

//     enum SemOpTimeState
//     {
//         PError,
//         Pok,
//         PTimeOver
//     };

public:
    // key_t  m_unix_key;
    // int    m_sem;
    // int    m_initVal;
    bool   m_createdFile;
    // bool   m_createdSem;

    string m_key;
    string m_fileName;


public:
    ZSystemSem();
    virtual ~ZSystemSem()
    {
        zprintf3("ZSystemSem destruct!\n");
        semFileCleanHandle();
    }

    inline string makeKeyFileName() const;

    key_t semFileHandle(AccessMode mode = Open);
    void semFileCleanHandle();
    // bool modifySemaphore(int count);
    // SemOpTimeState modifySemaphore(int count, int ms);
    int  createUnixKeyFile(const string &fileName);
    int  setKey(const string & key, int initVal = 0, AccessMode mode = Open);
    string key() const;
    // bool acquire();
    // bool release(int n = 1);
};

#endif // ZSYSTEMSEM_H
