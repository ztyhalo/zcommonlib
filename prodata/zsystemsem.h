#ifndef ZSYSTEMSEM_H
#define ZSYSTEMSEM_H

#include <string>
#include "lsystemsem.h"

using namespace std;

//通过生成文件生成sem key 类似于qt
class ZSystemSem:public LSystemSem
{

public:

    bool   m_createdFile;

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

    int  createUnixKeyFile(const string &fileName);
    int  setKey(const string & key, int initVal = 0, AccessMode mode = Open);
    string key() const;

};

#endif // ZSYSTEMSEM_H
