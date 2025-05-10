#ifndef ZBUFT_H
#define ZBUFT_H

#include "mutex_class.h"
#include "zprint.h"
#include <semaphore.h>
#include <zlockerclass.h>
#include <pthclass.h>

//数据buf操作类
template < class DTYPE, int N = 2 >
class Z_Buf_T:public MUTEX_CLASS
{
public:
    DTYPE           m_buf[N];
    uint            m_semWr;
    uint            m_semRd;
    int             m_size;
    sem_t           m_mgsem;


public:
    Z_Buf_T():m_semWr(0),m_semRd(0),m_size(0)
    {
        memset(m_buf, 0x00, sizeof(m_buf));
        sem_init(&m_mgsem, 0, 0);
    }
    virtual ~Z_Buf_T()
    {
        zprintf3("Z_Buf_T destruct!\n");

        int ret = sem_destroy(&m_mgsem);
        if(ret != 0)
        {
            zprintf1("Z_Buf_T destruct sem_t error %d!\n", ret);
            perror("Z_Buf_T sem_destory");
        }
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

    ZLockerClass<MUTEX_CLASS> locker(this);
    locker.lock();
    if(m_size >= N)
    {
        zprintf1("Z_Buf_T over\n");
        return;
    }
    m_buf[m_semWr] = val;
    m_semWr++;
    m_semWr %= N;
    m_size++;

    sem_post(&m_mgsem);
}

template < class DTYPE, int N >
DTYPE* Z_Buf_T< DTYPE, N >::buf_wr_data(const DTYPE &val)
{
    DTYPE* ret = NULL;

    ZLockerClass<MUTEX_CLASS> locker(this);
    locker.lock();

    if(m_size >= N)
    {
        zprintf1("Z_Buf_T over\n");
        return ret;
    }

    m_buf[m_semWr] = val;
    ret         = &m_buf[m_semWr];
    m_semWr++;
    m_semWr %= N;
    m_size++;

    return ret;
}

template < class DTYPE, int N >
void Z_Buf_T< DTYPE, N >::buf_write_data(const DTYPE * val)
{
    ZLockerClass<MUTEX_CLASS> locker(this);
    locker.lock();

    if(m_size >= N)
    {
        zprintf1("Z_Buf_T over\n");
        return;
    }

    m_buf[m_semWr] = *val;
    m_semWr++;
    m_semWr %= N;
    m_size++;

    sem_post(&m_mgsem);
}

template < class DTYPE, int N >
int Z_Buf_T< DTYPE, N >::buf_read_data(DTYPE& val)
{
    ZLockerClass<MUTEX_CLASS> locker(this);
    locker.lock();
    if(m_size <= 0)
        return -1;

    val = m_buf[m_semRd];
    m_semRd++;
    m_semRd %= N;
    m_size--;

    return 0;
}
template < class DTYPE, int N >
int Z_Buf_T< DTYPE, N >::wait_buf_sem(void)
{
    sem_wait(&m_mgsem);
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
            sem_post(&this->m_mgsem);
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
#endif // ZBUFT_H
