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
#include "pro_data.h"
#include <string.h>
#include "zlockerclass.h"
#include "zqtsharemem.h"
using namespace std;


// QT共享内存 不继承线程类
template < class T >
class QT_Share_MemT:public ZQTShareMem
{
public:
    T*      m_data;

  public:
    QT_Share_MemT():m_data(NULL)
    {
        ;
    }
    virtual ~QT_Share_MemT()
    {
        zprintf3("QT_Share_MemT destruct!\n");
    }

    // int  creat_data(int size);
    T*   creat_data(int size, const QString & keyid, AccessMode mode);
    void lock_qtshare(void);
    void unlock_qtshare(void);
    void set_data(T * addr, T  val);

};

template < class T >
void QT_Share_MemT< T >::lock_qtshare(void)
{
    lock();
}
template < class T >
void QT_Share_MemT< T >::unlock_qtshare(void)
{
    unlock();
}

// template < class T >
// int QT_Share_MemT< T >::creat_data(int size)
// {
//     if(this->newcreateData(size) == 0)
//         this->m_data = (T*)data();
//     else
//         zprintf1("QT_Share_MemT create size error!\n");
//     return 0;
// }

template < class T >
T* QT_Share_MemT< T >::creat_data(int size, const QString & keyid, AccessMode mode)
{
    this->m_data = (T *)createData(size, keyid, mode);
    if(this->m_data == NULL)
        zprintf1("QT_Share_MemT create %s error!\n", keyid.toStdString().c_str());
    return this->m_data;
}

template <class T>
void QT_Share_MemT<T>::set_data(T * addr, T  val)
{
    if(addr == NULL) return ;
    lock();
    *addr = val;
    unlock();
}

// QT共享内存 不继承线程类
template < class T >
class QTShareDataT :public ZQTShareMem
{
public:
    T*      m_data;
    int     m_size;      //表示为sizeof(T)*num;
private:
    int     m_classSize;
  public:
    QTShareDataT():m_data(NULL),m_size(0),m_classSize(0)
    {
        ;
    }
    virtual ~QTShareDataT()
    {
        zprintf3("QTShareDataT destruct!\n");
    }

    int  creat_data(int size);
    T*   creat_data(int size, const QString & keyid, AccessMode mode);
    int  read_creat_data(int size, const QString & keyid = "lhshare");
    void set_data(int add, const T & val) ;
    void set_data(T* addr,  T  val);
    T    get_data(int add) ;
    T    get_data(const T* addr);
    int  get_data(int add, T& val);
    int  get_data(const T* addr, T& val);
    void noblock_set_data(int add, T val);
    T    noblock_get_data(int add);
    int  noblock_get_data(int add, T& val);
    void lock_qtshare(void);
    void unlock_qtshare(void);
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
int QTShareDataT< T >::creat_data(int size)
{   
    if(this->newcreateData(size) == 0)
    {

        ZLockerClass<QTShareDataT< T >> locker(this);
        locker.lock();

        this->m_data = (T*) data();
        this->m_size =  this->size();
        if(size != this->m_size)
        {
            zprintf1("QTShareDataT size %d create size %d!\n", size, this->m_size);
        }
        m_classSize = (int)(this->m_size / sizeof(T));
    }
    else
        zprintf1("QTShareDataT create size error!\n");


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
        }
        m_classSize = (int)(this->m_size / sizeof(T));

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
        m_classSize = (int)(this->m_size / sizeof(T));
        return 0;
    }
    else
    {
        zprintf1("QTShareDataT read create %s error!\n", keyid.toStdString().c_str());
        return -2;
    }

}

template < class T >
void QTShareDataT< T >::set_data(int add, const T & val)
{
    if (add >= m_classSize)
    {
        zprintf1("set data off\n");
        return;
    }
    lock();
    memcpy(this->m_data + add, &val, sizeof(T));
    unlock();
}

template < class T >
void QTShareDataT< T >::set_data(T* addr,  T  val)
{
    if ((addr - this->m_data) >= m_classSize)
    {
        zprintf1("set data off\n");
        return;
    }
    lock();
    *addr = val;
    unlock();
}

template < class T >
T QTShareDataT< T >::get_data(int add)
{
    if (add >= this->m_size / sizeof(T))
    {
        printf("get data off\n");
        return *this->m_data;
    }
    T mid;
    lock();
    mid = *(this->m_data + add);
    unlock();
    return mid;
}

template < class T >
T QTShareDataT< T >::get_data(const T * addr)
{
    if ((addr - this->m_data) >= m_classSize)
    {
        printf("get data off\n");
        return *this->m_data;
    }
    T mid;
    lock();
    mid = *(addr);
    unlock();
    return mid;
}

template < class T >
int QTShareDataT< T >::get_data(int add, T & val)
{
    if (add >= m_classSize)
    {
        zprintf1("get data off\n");
        return -1;
    }

    lock();
    val = *(this->m_data + add);
    unlock();
    return 0;
}
template < class T >
int QTShareDataT< T >::get_data(const T * addr, T& val)
{
    if ((addr - this->m_data) >= m_classSize)
    {
        zprintf1("get data off\n");
        return -1;
    }

    lock();
    val = *(addr);
    unlock();
    return 0;
}

template < class T >
void QTShareDataT< T >::noblock_set_data(int add, T val)
{
    if (add >= m_classSize)
    {
        zprintf1("set data off\n");
        return;
    }
    memcpy(this->m_data + add, &val, sizeof(T));
}

template < class T >
T QTShareDataT< T >::noblock_get_data(int add)
{
    if (add >= m_classSize)
    {
        zprintf1("get data off\n");
        return *this->m_data;
    }
    T mid;
    mid = *(this->m_data + add);
    return mid;
}

template < class T >
int QTShareDataT< T >::noblock_get_data(int add, T & val)
{
    if (add >= m_classSize)
    {
        zprintf1("get data off\n");
        return -1;
    }
    val = *(this->m_data + add);
    return 0;
}

#endif /*__SHAREMEM_H__*/
