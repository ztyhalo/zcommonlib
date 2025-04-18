/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2017.7.11 将1个文件拆分为多个文件
 ****************************************************/

#ifndef __SEMSHARE_H__
#define __SEMSHARE_H__

#include <mutex>
#include "prodata/sharemem.h"
#include "syssem.h"

using namespace  std;

#define Z_MEM_ALIGEN_SIZE(size, align) (((size) +(align) - 1) & ~((align) -1))

// linux共享内存加信号 不继承线程类
template < class T >
class Sem_Share_Data : public ShareDataT< T >
{
  public:
    int   semid;
    int  bufsize;

    uint* rd_buf_p;
    uint* wr_buf_p;
    uint *  m_num_p;
    key_t sem_key;
    mutex m_lock;

  public:
    Sem_Share_Data():semid(0),m_num_p(NULL),sem_key(19860610),bufsize(0),rd_buf_p(NULL),wr_buf_p(NULL)
    {
        ;
    }
    explicit Sem_Share_Data(uint sz, key_t semkey = 19860610, key_t sharekey = 20130410)
    {
        creat_sem_data(sz, semkey, sharekey);
    }
    virtual ~Sem_Share_Data()
    {
        zprintf3("Sem_Share_Data destruct!\n");
    }
    int creat_sem_data(uint sz, key_t semkey = 19860610, key_t sharekey = 20130410);
    int write_send_data(const T & val);
    int read_send_data(T& val);
    int wait_thread_sem(void);
};

template < class T >
int Sem_Share_Data< T >::creat_sem_data(uint sz, key_t semkey, key_t sharekey)
{
    int   err  = 0;
    uint* midp = NULL;

    lock_guard<mutex> lock(m_lock);
    zprintf3("share key is %d\n", sharekey);

    int aline = Z_MEM_ALIGEN_SIZE(sz, 4);
    int alinesize = Z_MEM_ALIGEN_SIZE(aline + sizeof(uint) * 3, 4);


    err     = this->creat_data(alinesize, sharekey);
    bufsize = sz / sizeof(T);
    if (err == 0)
    {
        midp = (uint*)this->data;
        midp += aline/4;
        rd_buf_p  = midp;
        wr_buf_p  = midp + 1;
        m_num_p = midp + 2;
        *rd_buf_p = 0;
        *wr_buf_p = 0;
        *m_num_p = 0;
        semid     = create_sem(semkey, 0);
        if(semid <= 0)
        {
            zprintf1("Sem_Share_Data create_sem %d error !\n",semkey);
        }
        err       = semid > 0 ? 0 : -2;
    }
    return err;
}

template < class T >
int Sem_Share_Data< T >::write_send_data(const T & val)
{
    lock_guard<mutex> lock(m_lock);
    if(m_num >= bufsize)
    {
        zprintf1("write off \n");
        return -1;
    }
    uint mid = *wr_buf_p;

    this->set_data(mid, val);
    mid++;
    mid %= bufsize;
    *wr_buf_p = mid;
    m_num++;
    sem_v(semid);
    return 0;
}

template < class T >
int Sem_Share_Data< T >::read_send_data(T& val)
{
    lock_guard<mutex> lock(m_lock);
    uint mid = *rd_buf_p;

    if(m_num <= 0)
    {
        zprintf3("read send data error!\n");
        return -1;
    }

    this->get_data(mid, val);

    mid++;
    mid %= bufsize;

    *rd_buf_p = mid;
    m_num--;

    return 0;
}
template < class T >
int Sem_Share_Data< T >::wait_thread_sem(void)
{
    if (sem_p(semid) == 0)
        return 0;
    else
        return -1;
}

// linux共享内存加信号 继承线程类
template < class T, class FAT >
class Sem_Pth_Data : public Sem_Share_Data< T >, public Call_B_T< T, FAT >
{
  public:
    Sem_Pth_Data()
    {
        ;
    }
    virtual ~Sem_Pth_Data()
    {
          zprintf3("Sem_Pth_Data destruct!\n");
    }
    void run();
};

template < class T, class FAT >
void Sem_Pth_Data< T, FAT >::run(void)
{
    while (this->running)
    {
        if (sem_p(this->semid) == 0)
        {
            T val;
            if (read_send_data(val) == 0)
            {
                if (this->z_callbak != NULL) //执行操作
                {
                    this->z_callbak(this->father, val);
                }
            }
        }
    }
}
// qt共享内存加linux信号 不继承线程类
template < class T >
class Sem_Qt_Data : public QTShareDataT< T >
{
  public:
    int       semid;
    int      bufsize;
    int      m_num;
    uint16_t* rd_buf_p;
    uint16_t* wr_buf_p;
    key_t     sem_key;

  public:
    Sem_Qt_Data():semid(0),bufsize(0),m_num(0),rd_buf_p(NULL),wr_buf_p(NULL),sem_key(19860610)
    {
        ;
    }
    explicit Sem_Qt_Data(uint sz, key_t semkey = 19860610, const QString & sharekey = "lhshare")
    {
        creat_sem_data(sz, semkey, sharekey);
    }
    virtual ~Sem_Qt_Data()
    {
        zprintf3("destory Sem_Qt_Data!\n");
    }

    int creat_sem_data(uint sz, key_t semkey = 19860610, const QString & sharekey = "lhshare");
    int write_send_data(const T & val);
    int read_send_data(T& val);
    int wait_thread_sem(void);
};

template < class T >
int Sem_Qt_Data< T >::creat_sem_data(uint sz, key_t semkey, const QString & sharekey)
{
    int       err     = 0;
    uint16_t* midp    = NULL;
    T*        midaddr = NULL;

    zprintf3("semkey is %d!\n", semkey);
    midaddr = this->creat_data(sz + sizeof(uint16_t) * 2, sharekey);
    bufsize = sz / sizeof(T);
    if (midaddr != NULL)
    {
        midp      = (uint16_t*) (this->data + bufsize);
        wr_buf_p  = midp;
        rd_buf_p  = midp + 1;
        *rd_buf_p = 0;
        *wr_buf_p = 0;
        semid     = create_sem(semkey, 0);
        if(semid <= 0)
        {
            zprintf1("Sem_Qt_Data create_sem %d error !\n",semkey);
        }
        err       = semid > 0 ? 0 : -2;
        m_num = 0;
    }
    else
        err = -1;
    return err;
}

template < class T >
int Sem_Qt_Data< T >::write_send_data(const T & val)
{
    this->lock_qtshare();

    if(m_num >= bufsize)
    {
        zprintf1("write off \n");
        this->unlock_qtshare();
        return -1;
    }

    uint16_t mid = *wr_buf_p;

    this->noblock_set_data(mid, val);
    mid++;

    mid %= bufsize;
    *wr_buf_p = mid;
    m_num++;
    this->unlock_qtshare();
    sem_v(semid);
    return 0;
}

template < class T >
int Sem_Qt_Data< T >::read_send_data(T& val)
{
    this->lock_qtshare();

    if(m_num <= 0)
    {
        zprintf4("read send data error!\n");
        this->unlock_qtshare();
        return -1;
    }


    uint16_t mid = *rd_buf_p;

    this->noblock_get_data(mid, val);

    mid++;
    mid %= bufsize;

    *rd_buf_p = mid;
    m_num--;

    this->unlock_qtshare();
    return 0;
}
template < class T >
int Sem_Qt_Data< T >::wait_thread_sem(void)
{
    if (sem_p(semid) == 0)
        return 0;
    else
        return -1;
}

// qt共享内存加linux信号 继承线程类
template < class T, class FAT >
class Sem_QtPth_Data : public Sem_Qt_Data< T >, public Call_B_T< T, FAT >
{
  public:
    Sem_QtPth_Data()
    {
        ;
    }
    virtual ~Sem_QtPth_Data()
    {
        zprintf3("destory Sem_QtPth_Data!\n");
    }

    void run();
};

template < class T, class FAT >
void Sem_QtPth_Data< T, FAT >::run(void)
{
    zprintf4("qt sem run!\n");

    while (this->running)
    {
        if (sem_p(this->semid, 10) == 0)
        {
            T val;
            while (this->read_send_data(val) == 0)
            {
                if (this->z_callbak != NULL) //执行操作
                {
                    this->z_callbak(this->father, val);
                }
            }
        }
        else
        {
            pthread_testcancel();
        }
    }
    pthread_exit(NULL);
}

#endif /*__SEMSHARE_H__*/
