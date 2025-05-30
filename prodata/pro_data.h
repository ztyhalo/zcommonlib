/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.2
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2025-4-17 优化
 ****************************************************/

#ifndef PRO_DATA_H
#define PRO_DATA_H
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/msg.h>
#include <pthread.h>
#include <semaphore.h>
#include "zprint.h"

using namespace std;

#define LINUX_MSG_MAX 8192


template < class T >
class creatdata
{
  public:
    T*    m_data;
    int   m_size;      //表示为sizeof(T)*num;
    int   m_creatmark; //表示是自己创建的

  public:
    creatdata(T* addr = NULL, int sz = 0):m_data(addr),m_size(sz),m_creatmark(0)
    {
        ;
    }
    virtual ~creatdata()
    {

        if (m_data != NULL && m_creatmark == 1)
        {
            delete[] m_data;
            m_data = NULL;
            m_creatmark = 0;
            m_size = 0;
            zprintf3("creatdata delete");
        }
    }

    void         data_init(T* add = NULL, int size = 0);
    virtual int  creat_data(int size);
    virtual void set_data(uint add, const T & val);
    virtual T    get_data(uint add);
    virtual int  get_data(uint add, T & val);
};

template < class T >
void creatdata< T >::data_init(T* add, int size)
{
    if (m_data != NULL && m_creatmark == 1)
    {
        delete[] m_data;
        m_data = NULL;
        m_creatmark = 0;
    }
    m_data = add;
    m_size = size;
}

template < class T >
int creatdata< T >::creat_data(int size)
{
    m_data = new T[size];
    if (m_data != NULL)
    {
        m_creatmark = 1;
        m_size = size * sizeof(T);
        return 0;
    }
    return -1;
}

template < class T >
void creatdata< T >::set_data(uint add, const T & val)
{
    if (add >= m_size / sizeof(T))
    {
        zprintf1("creatdata set data off\n");
        return;
    }
    // data[add] = val;
    memcpy(m_data + add, &val, sizeof(T));
}

template < class T >
T creatdata< T >::get_data(uint add)
{
    if (add >= m_size / sizeof(T))
    {
        zprintf1("get data off\n");
        return *m_data;
    }
    return *(m_data + add);
}

template < class T >
int creatdata< T >::get_data(uint add, T & val)
{
    if (add >= m_size / sizeof(T))
    {
        zprintf1("get data off\n");
        return -1;
    }
    val = m_data[add];
    // val = *(data + add);
    return 0;
}




#endif /*PRO_DATA_H*/
