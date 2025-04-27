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
#include "epoll/e_poll.h"
#include "mutex_class.h"
#include "zlockerclass.h"
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
        zprintf1("set data off\n");
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

//线程回调类
template < class DTYPE, class F >
class Call_B_T : public Pth_Class
{
  public:
    F* father;

  public:
    Call_B_T();
    virtual ~Call_B_T()
    {
        zprintf3("destory Call_B_T!\n");
        this->running = 0;
    }

    int (*z_callbak)(F* pro, DTYPE val);

    int set_z_callback(int (*callback)(F* pro, DTYPE), F* arg);
    int z_pthread_init(int (*callback)(F* pro, DTYPE), F* arg, const string & pthName="");
};

template < class DTYPE, class F >
Call_B_T< DTYPE, F >::Call_B_T():father(NULL),z_callbak(NULL)
{
    ;
}
template < class DTYPE, class F >
int Call_B_T< DTYPE, F >::set_z_callback(int (*callback)(F* pro, DTYPE), F* arg)
{
    if (callback != NULL)
    {
        z_callbak = callback;
        father    = arg;
        return 0;
    }
    return -1;
}

template < class DTYPE, class F >
int Call_B_T< DTYPE, F >::z_pthread_init(int (*callback)(F* pro, DTYPE), F* arg, const string & pthName)
{
    set_z_callback(callback, arg);
    start(pthName);
    return 0;
}
//数据buf操作类
template < class DTYPE, int N = 2 >
class Z_Buf_T:public MUTEX_CLASS
{
  public:
    DTYPE           buf[N];
    uint            sem_wr;
    uint            sem_rd;
    sem_t           mgsem;
    // pthread_mutex_t sem_mut;

  public:
    Z_Buf_T():sem_wr(0),sem_rd(0)
    {
        memset(buf, 0x00, sizeof(buf));
        sem_init(&mgsem, 0, 0);
        // pthread_mutex_init(&sem_mut, NULL);
    }
    virtual ~Z_Buf_T()
    {
        zprintf3("Z_Buf_T destruct!\n");
        // pthread_mutex_destroy(&sem_mut);
    }

    void   buf_write_data(const DTYPE & val);
    DTYPE* buf_wr_data(const DTYPE & val);
    void   buf_write_data(const DTYPE* val);
    int    buf_read_data(DTYPE & val);
    int    wait_buf_sem(void);
};

template < class DTYPE, int N >
void Z_Buf_T< DTYPE, N >::buf_write_data(const DTYPE & val)
{
    // pthread_mutex_lock(&sem_mut);
    ZLockerClass<MUTEX_CLASS> locker(this);
    locker.lock();
    buf[sem_wr] = val;
    sem_wr++;
    sem_wr %= N;

    if (sem_wr == sem_rd)
    {
        zprintf1("sembuf_t over\n");
    }
    // pthread_mutex_unlock(&sem_mut);
    sem_post(&mgsem);
}

template < class DTYPE, int N >
DTYPE* Z_Buf_T< DTYPE, N >::buf_wr_data(const DTYPE &val)
{
    DTYPE* ret = NULL;

    ZLockerClass<MUTEX_CLASS> locker(this);
    locker.lock();
    buf[sem_wr] = val;
    ret         = &buf[sem_wr];
    sem_wr++;
    sem_wr %= N;

    if (sem_wr == sem_rd)
    {
        zprintf1("sembuf_t over\n");
    }
    return ret;
}

template < class DTYPE, int N >
void Z_Buf_T< DTYPE, N >::buf_write_data(const DTYPE * val)
{
    ZLockerClass<MUTEX_CLASS> locker(this);
    locker.lock();
    buf[sem_wr] = *val;
    sem_wr++;
    sem_wr %= N;

    if (sem_wr == sem_rd)
    {
        zprintf1("sembuf_t over\n");
    }
    // pthread_mutex_unlock(&sem_mut);
    sem_post(&mgsem);
}

template < class DTYPE, int N >
int Z_Buf_T< DTYPE, N >::buf_read_data(DTYPE& val)
{
    ZLockerClass<MUTEX_CLASS> locker(this);
    locker.lock();
    if (sem_rd == sem_wr)
    {
        // pthread_mutex_unlock(&sem_mut);
        return -1;
    }
    val = buf[sem_rd];
    sem_rd++;
    sem_rd %= N;
    // pthread_mutex_unlock(&sem_mut);
    return 0;
}
template < class DTYPE, int N >
int Z_Buf_T< DTYPE, N >::wait_buf_sem(void)
{
    sem_wait(&mgsem);
    return 0;
}

//带线程回调的buf操作类
template < class DTYPE, int N = 2, class F = void >
class Pth_Buf_T : public Z_Buf_T< DTYPE, N >, public Call_B_T< DTYPE, F >
{
  public:
    Pth_Buf_T()
    {
        ;
    }
    virtual ~Pth_Buf_T()
    {
        zprintf3("Pth_Buf_T delete!\n");
        if(this->running)
        {
            this->running = 0;
            sem_post(&this->mgsem);
            this->waitEnd();
        }
    }
    void run();
};

template < class DTYPE, int N, class F >
void Pth_Buf_T< DTYPE, N, F >::run(void)
{
    while (this->running)
    {
        if (this->wait_buf_sem() == 0)
        {
            DTYPE val;
            if (this->buf_read_data(val) == 0)
            {
                if (this->z_callbak != NULL) //执行操作
                {
                    this->z_callbak(this->father, val);
                }
            }
        }
    }
}

#endif /*PRO_DATA_H*/
