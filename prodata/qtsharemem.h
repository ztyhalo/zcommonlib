/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2017.7.11 将1个文件拆分为多个文件
 *         2025.4.18 优化
 *
 ****************************************************/

#ifndef __QTSHAREMEM_H__
#define __QTSHAREMEM_H__

#include <QVector>
#include <QString>
#include <QSharedMemory>

#include <string.h>
#include "zlockerclass.h"
#include "zqtsharemem.h"
#include "sharedateprocessT.h"
using namespace std;


// QT共享内存 不继承线程类
template < class T >
class QTShareDataT :public ZQTShareMem, public ShareDataProcessT<T>
{
  public:
    QTShareDataT()
    {
        ;
    }
    virtual ~QTShareDataT()
    {
        zprintf3("QTShareDataT destruct!\n");
    }

    T*   creat_data(int size, const QString & keyid, AccessMode mode);
    int  create_data(int size, const QString & keyid);
    int  read_creat_data(int size, const QString & keyid = "lhshare");
    int  createReadData(const QString & keyid = "lhshare");

    void lock_qtshare(void);
    void unlock_qtshare(void);
    bool dateLock(void) override;
    bool  dateUnlock(void) override;
};

template < class T >
void QTShareDataT< T >::lock_qtshare(void)
{
    lock();
}
template < class T >
void QTShareDataT< T >::unlock_qtshare(void)
{
    unlock();
}

template < class T >
bool QTShareDataT< T >::dateLock(void)
{
    return this->lock();
}
template < class T >
bool QTShareDataT< T >::dateUnlock(void)
{
    return this->unlock();
}

template < class T >
int  QTShareDataT< T >::create_data(int size, const QString & keyid)
{
    this->m_data = (T*)createData(size, keyid, Create);
    if(this->m_data == NULL)
    {
        zprintf1("QTShareDataT create %s error!\n", keyid.toStdString().c_str());
        return -1;
    }
    else
    {
        ZLockerClass<QTShareDataT< T >> locker(this);
        locker.lock();

        this->m_size =  this->size();

        if(size != this->m_size)
        {
            zprintf1("QTShareDataT size %d create size %d!\n", size, this->m_size);
            return -2;
        }
        this->m_classSize = (int)(this->m_size / sizeof(T));
    }
    return 0;
}

template < class T >
T* QTShareDataT< T >::creat_data(int size, const QString & keyid, AccessMode mode)
{

    this->m_data = (T *)createData(size, keyid, mode);
    if(this->m_data == NULL)
        zprintf1("QTShareDataT create %s error!\n", keyid.toStdString().c_str());
    else
    {

        ZLockerClass<QTShareDataT< T >> locker(this);
        locker.lock();

        this->m_size =  this->size();

        if(size != this->m_size)
        {
            zprintf1("QTShareDataT size %d create size %d!\n", size, this->m_size);
            return NULL;
        }
        this->m_classSize = (int)(this->m_size / sizeof(T));

    }
    return this->m_data;
}

template < class T >
int QTShareDataT< T >::read_creat_data(int size, const QString & keyid)
{

    if(readCreateData(size, keyid) == 0)
    {
        ZLockerClass<QTShareDataT< T >> locker(this);
        locker.lock();
        this->m_data = (T*)data();
        this->m_size = this->size();

        if(size != this->m_size)
        {
            zprintf1("QTShareDataT read create size %d create size %d!\n", size, this->m_size);
            return -1;
        }
        this->m_classSize = (int)(this->m_size / sizeof(T));
        return 0;
    }
    else
    {
        zprintf1("QTShareDataT read create %s error!\n", keyid.toStdString().c_str());
        return -2;
    }

}
template < class T >
int QTShareDataT< T >::createReadData(const QString & keyid)
{

    if(readCreate(keyid) == 0)
    {
        ZLockerClass<QTShareDataT< T >> locker(this);
        locker.lock();
        this->m_data = (T*)data();
        this->m_size = this->size();

        this->m_classSize = (int)(this->m_size / sizeof(T));
        return 0;
    }
    else
    {
        zprintf1("QTShareDataT read create %s error!\n", keyid.toStdString().c_str());
        return -2;
    }

}

#endif /*__SHAREMEM_H__*/
