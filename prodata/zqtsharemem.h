#ifndef ZQTSHAREMEM_H
#define ZQTSHAREMEM_H

#include <QVector>
#include <QString>
#include <QSharedMemory>
#include "zprint.h"
// #include "mutex_class.h"
#include "zsysshmbase.h"

using namespace std;

class ZQTShareMem:public ZSysShmBase
{

public:
    QSharedMemory m_lhshare;
    QString       m_shmKey;
    // bool          m_createShm;
    // MUTEX_CLASS   m_mutex;

public:
    ZQTShareMem():m_shmKey("") //,m_createShm(false)
    {
        m_lhshare.setKey(m_shmKey);
    }
    virtual ~ZQTShareMem()
    {
        zprintf3("destory ZQTShareMem!\n");
        if (m_lhshare.isAttached())
        {
            zprintf3("qt share have attach!\n");
            m_lhshare.detach();
        }
        if(m_createShm)
        {
            zprintf3("ZQTShareMem create release!\n");
            destory();
            m_createShm = false;
        }
    }
    bool    destory();
    int     size() const override;
    void    *data() override;
    int     newcreateData(int size);
    void*   createData(int size, const QString & keyid, AccessMode mode = Open);

    // bool    lock(void);
    // bool    unlock(void);
    bool semLock() override;
    bool semUnlock() override;
    int     readCreateData(int size, const QString & keyid);

};

#endif // ZQTSHAREMEM_H
