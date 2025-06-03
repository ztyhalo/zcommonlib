#ifndef TWO_BUF_PTHT_H
#define TWO_BUF_PTHT_H

#include <sys/types.h>
// #include "zprint/zprint.h"
#include <mutex>
#include "mutex/mutex_class.h"
#include <semaphore.h>
#include <pthclass.h>


template<class DTYPE, int N = 2, int SIZE = 2048>
class TwoBufT //:public MUTEX_CLASS
{
  public:
    DTYPE               m_buf[N][SIZE];
    int                 m_size[N];
    int                 m_wr;
    int                 m_rd;
    int                 m_num;
    int                 m_maxNum;
    sem_t               m_sem;
    MUTEX_CLASS         m_mutex;


  public:
    explicit TwoBufT():m_wr(0),m_rd(0),m_num(0),m_maxNum(0)
    {
        memset(m_size, 0x00, sizeof(m_size));
        memset(m_buf, 0x00, sizeof(m_buf));
        sem_init(&m_sem, 0, 0);
    }
    virtual ~TwoBufT()
    {
        printf("TwoBufT destruct!\n");
    }

    int writeBufData(DTYPE * val, int num);
    int readBufData(DTYPE * val, int num);
    // int add_buf_rd(void);

};

template<class DTYPE, int N, int SIZE>
int TwoBufT<DTYPE, N, SIZE>::writeBufData(DTYPE * val, int num)
{
    lock_guard<MUTEX_CLASS> locker(m_mutex);
    if(m_num >= N)
    {
        printf("Write buf over!\n");
        return -1;
    }
    if(sizeof(DTYPE)*num > SIZE)
    {
        printf("Write data over!\n");
        return -2;
    }
    memcpy(m_buf[m_wr], val, sizeof(DTYPE)*num);
    m_size[m_wr] = sizeof(DTYPE)*num;

    m_wr++;
    m_wr %= N;
    m_num++;
    if(m_num > m_maxNum)
    {
        m_maxNum = m_num;
        printf("buf max write is %d!\n", m_maxNum);
    }
    sem_post(&m_sem);
    return 0;
}


template<class DTYPE, int N, int SIZE>
int TwoBufT<DTYPE,N,SIZE>::readBufData(DTYPE * addr, int num)
{


    lock_guard<MUTEX_CLASS> locker(m_mutex);
    if(m_num > 0 && addr != NULL)
    {
        int size = 0;
        if(num >= m_size[m_rd])
        {
            memcpy(addr, m_buf[m_rd], m_size[m_rd]);
            size = m_size[m_rd];
        }
        else
        {
            memcpy(addr, m_buf[m_rd], num);
            size = num;
        }
        m_rd++;
        m_rd %= N;
        m_num--;
        return size;
    }
    else
        return 0;

}


//带线程回调的buf操作类
template < class DTYPE, int N = 2, int SIZE = 2048, class F = void >
class TwoBufPthT : public TwoBufT< DTYPE, N ,SIZE>,public CallBackPoint_T<DTYPE, F>,public Pth_Class
{
  public:
    TwoBufPthT()
    {
        ;
    }
    virtual ~TwoBufPthT()
    {
        printf("TwoBufPthT delete!\n");
        if(this->running)
        {
            this->running = 0;
            sem_post(&this->m_sem);
            this->waitEnd();
        }
    }
    void run();
};

template < class DTYPE, int N, int SIZE, class F >
void TwoBufPthT< DTYPE, N,  SIZE, F >::run(void)
{
    while (this->running)
    {
        if (sem_wait(&this->m_sem)== 0)
        {
            DTYPE val[SIZE];
            int size = this->readBufData(val, SIZE);
            if (size > 0)
            {
                if (this->m_callbak != NULL) //执行操作
                {
                    this->m_callbak(this->m_father, val, size);
                }
            }
        }
        else
            break;
    }
}


#endif
