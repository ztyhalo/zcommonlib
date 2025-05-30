#ifndef Z_BUF_MODEL_H
#define Z_BUF_MODEL_H

#include <sys/types.h>
#include "zprint/zprint.h"
#include <mutex>
#include "mutex/mutex_class.h"
#include <semaphore.h>
#include <pthclass.h>

template<class DTYPE,class SUPPLEM, int N = 2, int SIZE = 2048>
class ZBufModel:public MUTEX_CLASS
{
public:
    DTYPE                m_buf[N][SIZE];
    uint                 m_size[N];
    uint                 m_wr;
    uint                 m_rd;
    uint                 m_num;
    uint                 m_maxnum;
    SUPPLEM              m_supm[N];

public:
    explicit ZBufModel():m_wr(0),m_rd(0),m_num(0),m_maxnum(0)
    {
        memset(m_size, 0x00, sizeof(m_size));
        memset(m_buf, 0x00, sizeof(m_buf));
        memset(m_supm,0x00, sizeof(m_supm));
    }
    virtual ~ZBufModel()
    {
        zprintf3("ZBufModel destruct!\n");
    }

    int buf_basewrite_data(DTYPE * val, int num, SUPPLEM para);
    int buf_write_data(DTYPE * val, int num, SUPPLEM para);
    int buf_write_data_from_file(FILE * fp, int num, SUPPLEM para);
    int get_buf_data(DTYPE & addr, SUPPLEM * para);
    int add_buf_rd(void);

};

template<class DTYPE, class SUPPLEM,int N, int SIZE>
int ZBufModel<DTYPE,SUPPLEM,N,SIZE>::buf_basewrite_data(DTYPE * val, int num, SUPPLEM para)
{
    if(m_num >= N)
    {
        zprintf1("Write buf over!\n");
        return -1;
    }
    if(sizeof(DTYPE)*num > SIZE)
    {
        zprintf1("Write data over!\n");
        return -2;
    }
    memcpy(m_buf[m_wr], val, sizeof(DTYPE)*num);
    m_size[m_wr] = sizeof(DTYPE)*num;
    m_supm[m_wr] = para;
    m_wr++;
    m_wr %= N;
    m_num++;
    if(m_num > m_maxnum)
    {
        m_maxnum = m_num;
        zprintf1("buf max write is %d!\n", m_maxnum);
    }

    return 0;
}

template<class DTYPE, class SUPPLEM, int N, int SIZE>
int ZBufModel<DTYPE,SUPPLEM,N,SIZE>::buf_write_data(DTYPE * val, int num, SUPPLEM para)
{
    int err;
    lock();
    err = buf_basewrite_data(val, num, para);
    unlock();
    return err;
}
template<class DTYPE, class SUPPLEM, int N, int SIZE>
int ZBufModel<DTYPE,SUPPLEM,N,SIZE>::buf_write_data_from_file(FILE * fp, int num, SUPPLEM para)
{
    int err = 0;
    int lssize = 0;

    if(fp == NULL)
        return -1;
    lock();

    if(m_num >= N)
    {
        zprintf1("Write buf over!\n");
        err =  -2;
        goto WRITEEND;
    }
    if(num > SIZE)
    {
        zprintf1("Write data over!\n");
        err = -3;
        goto WRITEEND;
    }

    err = fread(m_buf[m_wr], 1, num, fp);

    if(err != num)
    {
        zprintf1("zty file read %d error!\n", err);
        err = -5;
        goto WRITEEND;
    }

    lssize = err -4;
    m_buf[m_wr][0] = m_buf[m_wr][1] = 0;
    m_buf[m_wr][2] = lssize >> 8;
    m_buf[m_wr][3] = lssize & 0xff;
    m_size[m_wr] = err;
    m_supm[m_wr] = para;
    m_wr++;
    m_wr %= N;
    m_num++;

WRITEEND:
    unlock();

    return err;
}

template<class DTYPE, class SUPPLEM, int N, int SIZE>
int ZBufModel<DTYPE,SUPPLEM,N,SIZE>::get_buf_data(DTYPE & addr, SUPPLEM * para)
{
    int size = 0;

    lock();
    if(m_num > 0)
    {
        addr = m_buf[m_rd];
        size = m_size[m_rd];
        *para = m_supm[m_rd];
    }
    unlock();

    return size;

}

template<class DTYPE, class SUPPLEM, int N, int SIZE>
int ZBufModel<DTYPE,SUPPLEM,N,SIZE>::add_buf_rd(void)
{
    int err = 0;
    lock();
    if(m_num >0)
    {
        m_rd++;
        m_rd %= N;
        m_num--;
    }
    else
    {
        zprintf1("zbufmodel add buf rd error!\n");
        err = -1;
    }
    unlock();
    return err;
}

template<class DTYPE, int N = 2, int SIZE = 2048>
class ZBufBaseT:public MUTEX_CLASS
{
  public:
    DTYPE               m_buf[N][SIZE];
    int                 m_size[N];
    int                 m_wr;
    int                 m_rd;
    int                 m_num;
    int                 m_maxNum;
    sem_t               m_sem;



  public:
    explicit ZBufBaseT():m_wr(0),m_rd(0),m_num(0),m_maxNum(0)
    {
        memset(m_size, 0x00, sizeof(m_size));
        memset(m_buf, 0x00, sizeof(m_buf));
        sem_init(&m_sem, 0, 0);
    }
    virtual ~ZBufBaseT()
    {
        zprintf3("ZBufBaseT destruct!\n");
    }

    int writeBufData(DTYPE * val, int num);
    int readBufData(DTYPE * val, int num);
    // int add_buf_rd(void);

};

template<class DTYPE, int N, int SIZE>
int ZBufBaseT<DTYPE, N, SIZE>::writeBufData(DTYPE * val, int num)
{
    lock_guard<MUTEX_CLASS> locker(this);
    if(m_num >= N)
    {
        zprintf1("Write buf over!\n");
        return -1;
    }
    if(sizeof(DTYPE)*num > SIZE)
    {
        zprintf1("Write data over!\n");
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
        zprintf1("buf max write is %d!\n", m_maxNum);
    }
    sem_post(&m_sem);
    return 0;
}


template<class DTYPE, int N, int SIZE>
int ZBufBaseT<DTYPE,N,SIZE>::readBufData(DTYPE * addr, int num)
{


    lock_guard<MUTEX_CLASS> locker(this);
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
class TwoBufPthT : public ZBufBaseT< DTYPE, N ,SIZE>, public Call_B_T< DTYPE, F >
{
  public:
    TwoBufPthT()
    {
        ;
    }
    virtual ~TwoBufPthT()
    {
        zprintf3("TwoBufPthT delete!\n");
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
            if (this->readBufData(val, SIZE) > 0)
            {
                if (this->z_callbak != NULL) //执行操作
                {
                    this->z_callbak(this->father, val);
                }
            }
        }
        else
            break;
    }
}


#endif
