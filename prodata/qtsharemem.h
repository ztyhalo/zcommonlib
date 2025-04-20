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

using namespace std;


// QT共享内存 不继承线程类
template < class T >
class QT_Share_MemT
{
  private:
    QSharedMemory lhshare;

  public:
    QString shm_key;
    T*      data;

  public:
    QT_Share_MemT():shm_key("lhshare")
    {
        data = NULL;
        lhshare.setKey(shm_key);
    }
    virtual ~QT_Share_MemT()
    {
        zprintf3("destory qt share mem!\n");
        if (lhshare.isAttached())
        {
            zprintf3("qt share have attach!\n");
            lhshare.detach();
        }
    }

    int  creat_data(int size);
    T*   creat_data(int size, const QString & keyid);
    void lock_qtshare(void);
    void unlock_qtshare(void);
};

template < class T >
void QT_Share_MemT< T >::lock_qtshare(void)
{
    lhshare.lock();
}
template < class T >
void QT_Share_MemT< T >::unlock_qtshare(void)
{
    lhshare.unlock();
}

template < class T >
int QT_Share_MemT< T >::creat_data(int siz)
{

    lhshare.setKey(shm_key);
    if (lhshare.isAttached())
    {
        zprintf2("qt share have attach!\n");
        lhshare.detach();
    }

    if (!lhshare.create(siz))
    {
        zprintf2("qt share creat fail!\n");

        if (lhshare.error() == 4) //已经存在
        {
            if (!lhshare.attach())
            {
                zprintf2("can't attatch qt share!\n");
                return -2;
            }
            if (lhshare.isAttached())
            {
               zprintf2("qt share test attach!\n");
            }
        }
        else
            return -1;
    }

    this->data = (T*) lhshare.data();

    return 0;
}

template < class T >
T* QT_Share_MemT< T >::creat_data(int size, const QString & keyid)
{
    shm_key = keyid;
    if (creat_data(size) == 0)
        return this->data;
    return NULL;
}

// QT共享内存 不继承线程类
template < class T >
class QTShareDataT : public creatdata< T >
{
  private:
    QSharedMemory lhshare;

  public:
    QString shm_key;

  public:
    QTShareDataT():shm_key("lhshare")
    {
        lhshare.setKey(shm_key);
    }
    virtual ~QTShareDataT()
    {

        if (this->data != NULL && this->creatmark == 1)
        {
            lhshare.detach();
            this->data      = NULL;
            this->creatmark = 0;
        }
        zprintf3("destory QTShareDataT!\n");
    }

    int  creat_data(int size) override;
    T*   creat_data(int size, const QString & keyid);
    int  read_creat_data(int size, const QString & keyid = "lhshare");
    void set_data(uint add, const T & val)override;
    void set_data(T* addr, const T & val);
    T    get_data(uint add) override;
    T    get_data(const T* addr);
    int  get_data(uint add, T& val) override;
    int  get_data(const T* addr, T& val);
    void noblock_set_data(uint add, T val);
    T    noblock_get_data(uint add);
    int  noblock_get_data(uint add, T& val);
    void lock_qtshare(void);
    void unlock_qtshare(void);
};

template < class T >
void QTShareDataT< T >::lock_qtshare(void)
{
    lhshare.lock();
}
template < class T >
void QTShareDataT< T >::unlock_qtshare(void)
{
    lhshare.unlock();
}

template < class T >
int QTShareDataT< T >::creat_data(int siz)
{

    lhshare.setKey(shm_key);
    if (lhshare.isAttached())
    {
        zprintf2("qt share have attach!\n");
        lhshare.detach();
    }

    if (!lhshare.create(siz))
    {
        zprintf2("qt share creat fail!\n");

        if (lhshare.error() == 4) //已经存在
        {
            if (!lhshare.attach())
            {
                zprintf2("can't attatch qt share!\n");
                return -2;
            }
        }
        else
            return -1;
    }

    this->creatmark = 1;

    this->data = (T*) lhshare.data();
    this->size = siz;
    // this->addp = this->data;

    return 0;
}

template < class T >
T* QTShareDataT< T >::creat_data(int size, const QString & keyid)
{
    shm_key = keyid;
    if (creat_data(size) == 0)
        return this->data;
    return NULL;
}

template < class T >
int QTShareDataT< T >::read_creat_data(int siz, const QString & keyid)
{
    shm_key = keyid;

    lhshare.setKey(shm_key);

    if (!lhshare.attach())
    {
        zprintf1("can't attatch qt share\n");
        return -1;
    }
    this->data = (T*) lhshare.data();
    this->size = siz;

    return 0;
}

template < class T >
void QTShareDataT< T >::set_data(uint add, const T & val)
{
    if (add >= this->size / sizeof(T))
    {
        zprintf1("set data off\n");
        return;
    }
    lhshare.lock();
    memcpy(this->data + add, &val, sizeof(T));
    lhshare.unlock();
}

template < class T >
void QTShareDataT< T >::set_data(T* addr, const T & val)
{
    if ((addr - this->data) >= this->size / sizeof(T))
    {
        zprintf1("set data off\n");
        return;
    }
    lhshare.lock();
    *addr = val;
    lhshare.unlock();
}

template < class T >
T QTShareDataT< T >::get_data(uint add)
{
    if (add >= this->size / sizeof(T))
    {
        printf("get data off\n");
        return *this->data;
    }
    T mid;
    lhshare.lock();
    mid = *(this->data + add);
    lhshare.unlock();
    return mid;
}

template < class T >
T QTShareDataT< T >::get_data(const T * addr)
{
    if ((addr - this->data) >= this->size / sizeof(T))
    {
        printf("get data off\n");
        return *this->data;
    }
    T mid;
    lhshare.lock();
    mid = *(addr);
    lhshare.unlock();
    return mid;
}

template < class T >
int QTShareDataT< T >::get_data(uint add, T & val)
{
    if (add >= this->size / sizeof(T))
    {
        zprintf1("get data off\n");
        return -1;
    }

    lhshare.lock();
    val = *(this->data + add);
    lhshare.unlock();
    return 0;
}
template < class T >
int QTShareDataT< T >::get_data(const T * addr, T& val)
{
    if ((addr - this->data) >= this->size / sizeof(T))
    {
        zprintf1("get data off\n");
        return -1;
    }

    lhshare.lock();
    val = *(addr);
    lhshare.unlock();
    return 0;
}

template < class T >
void QTShareDataT< T >::noblock_set_data(uint add, T val)
{
    if (add >= this->size / sizeof(T))
    {
        zprintf1("set data off\n");
        return;
    }
    memcpy(this->data + add, &val, sizeof(T));
}

template < class T >
T QTShareDataT< T >::noblock_get_data(uint add)
{
    if (add >= this->size / sizeof(T))
    {
        zprintf1("get data off\n");
        return *this->data;
    }
    T mid;
    mid = *(this->data + add);
    return mid;
}

template < class T >
int QTShareDataT< T >::noblock_get_data(uint add, T& val)
{
    if (add >= this->size / sizeof(T))
    {
        zprintf1("get data off\n");
        return -1;
    }
    val = *(this->data + add);
    return 0;
}

#endif /*__SHAREMEM_H__*/
