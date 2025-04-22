/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2017.7.11 将1个文件拆分为多个文件
 ****************************************************/

#ifndef __SEMSHARE_H__
#define __SEMSHARE_H__

// #include <mutex>
#include "prodata/sharemem.h"
#include "syssem.h"

using namespace  std;


// linux共享内存加信号 不继承线程类
template < class T >
class Sem_Share_Data : public ShareDataT< T >
{
  public:
    int     m_semid;
    int     m_bufsize;
    int     m_created;

    int *   m_pRd;
    int *   m_pWr;
    int *   m_num_p;
    key_t   m_semKey;

  public:
    Sem_Share_Data():m_semid(-1),m_created(0),m_num_p(NULL),
          m_semKey(19860610),m_bufsize(0),m_pRd(NULL),m_pWr(NULL)
    {
        ;
    }
    explicit Sem_Share_Data(uint size, key_t semkey = 19860610, key_t sharekey = 20130410)
    {
        creat_sem_data(size, semkey, sharekey);
    }
    virtual ~Sem_Share_Data()
    {
        zprintf3("Sem_Share_Data destruct!\n");
        cleanHandle();

    }
    void cleanHandle()
    {
        if(m_created)
        {
            if(-1 != m_semid)
            {
                if(-1 == semctl(m_semid, 0, IPC_RMID, 0))
                {
                    zprintf1("Sem_Share_Data rm msem %d error!\n", m_semid);
                }
                m_semid = -1;
            }
            m_created = 0;
        }
    }
    int creat_sem_data(uint size, key_t semkey = 19860610, key_t sharekey = 20130410);
    int write_send_data(const T & val);
    int read_send_data(T& val);
    int wait_thread_sem(void);
};

template < class T >
int Sem_Share_Data< T >::creat_sem_data(uint size, key_t semkey, key_t sharekey)
{
    int   err  = 0;
    int* midp = NULL;

    zprintf3("share key is %d\n", sharekey);
    cleanHandle();

    int aline = Z_MEM_ALIGEN_SIZE(size, 4);
    int alinesize = Z_MEM_ALIGEN_SIZE(aline + sizeof(int) * 3, 4);


    err     = this->creat_data(alinesize, sharekey);
    m_bufsize = size / sizeof(T);
    if (err == 0)
    {
        ZLockerClass<Sem_Share_Data< T >> locker(this);
        locker.lock();
        midp = (int*)this->m_memory;
        midp += aline/4;
        m_pRd  = midp;
        m_pWr  = midp + 1;
        m_num_p = midp + 2;
        *m_pRd = 0;
        *m_pWr = 0;
        *m_num_p = 0;

        m_semid     = new_create_sem(semkey, 0, m_created);
        if(m_semid <= 0)
        {
            zprintf1("Sem_Share_Data create_sem %d error !\n",semkey);
        }
        else
            m_semKey = semkey;
        err = m_semid > 0 ? 0 : -2;
    }
    else
        zprintf1("Sem_Share_Data create sharekey %d error !\n",sharekey);
    return err;
}

template < class T >
int Sem_Share_Data< T >::write_send_data(const T & val)
{

    ZLockerClass<Sem_Share_Data< T >> locker(this);
    locker.lock();
    int count = *m_num_p;
    if(count >= m_bufsize)
    {
        zprintf1("write off \n");
        return -1;
    }
    int mid = *m_pWr;

    this->set_data(mid, val);
    mid++;
    mid %= m_bufsize;
    *m_pWr = mid;
    count++;
    *m_num_p = count;
    sem_v(m_semid);
    return 0;
}

template < class T >
int Sem_Share_Data< T >::read_send_data(T & val)
{
    ZLockerClass<Sem_Share_Data< T >> locker(this);
    locker.lock();
    int count = *m_num_p;
    if(count <= 0)
    {
        zprintf3("read send data error!\n");
        return -1;
    }
    int mid = *m_pRd;

    this->get_data(mid, val);

    mid++;
    mid %= m_bufsize;

    *m_pRd = mid;
    count--;
    *m_num_p = count;

    return 0;
}
template < class T >
int Sem_Share_Data< T >::wait_thread_sem(void)
{
    if (sem_p(m_semid) == 0)
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
        this->running = 0;
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

#endif /*__SEMSHARE_H__*/
