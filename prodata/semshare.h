/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2017.7.11 将1个文件拆分为多个文件
 ****************************************************/

#ifndef __SEMSHARE_H__
#define __SEMSHARE_H__


#include "sharemem.h"
#include "syssem.h"
#include "zlockerclass.h"

#include "pthclass.h"
#include "lsystemsem.h"
#include "zsharememrw.h"
using namespace  std;


// linux共享内存加信号 不继承线程类
template < class T >
class Sem_Share_Data : public ShareDataT< T >,public ZSharememRW
{
public:
    // enum AccessMode
    // {
    //     Open,
    //     Create
    // };
  public:
    Sem_Share_Data()
    {
        ;
    }
    explicit Sem_Share_Data(uint size, key_t semkey = 19860610, key_t sharekey = 20130410)
    {
        creat_sem_data(size, semkey, sharekey, ZSysShmBase::Open);
    }
    virtual ~Sem_Share_Data()
    {
        zprintf3("Sem_Share_Data destruct!\n");

    }

    int creat_sem_data(uint size, key_t semkey, key_t sharekey , ZSysShmBase::AccessMode mode);
    int write_send_data(const T & val);
    int read_send_data(T& val);
};

template < class T >
int Sem_Share_Data< T >::creat_sem_data(uint size, key_t semkey, key_t sharekey, ZSysShmBase::AccessMode mode)
{
    int   err  = 0;
    int* midp = NULL;

    zprintf3("share key is %d sem key 0x%x:%d\n", sharekey, semkey, semkey);
    cleanHandle();

    int aline = Z_MEM_ALIGEN_SIZE(size, 4);
    int alinesize = Z_MEM_ALIGEN_SIZE(aline + sizeof(int) * 3, 4);


    err     = this->shareCreateData(alinesize, sharekey, mode);
    m_bufSize = size / sizeof(T);
    if (err == 0)
    {
        ZLockerClass<Sem_Share_Data< T >> locker(this);
        locker.lock();
        midp = (int*)this->m_memory;
        midp += aline/4;
        m_pRd  = midp;
        m_pWr  = midp + 1;
        m_pSize = midp + 2;
        *m_pRd = 0;
        *m_pWr = 0;
        *m_pSize = 0;

        if(m_sem.setKey(semkey, 0,  (LSystemSem::AccessMode)mode) != 0)
        {
            zprintf1("Sem_Qt_Data create_sem %d error !\n",semkey);
            err = -2;
        }
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
    int count = *m_pSize;
    if(count >= m_bufSize)
    {
        zprintf1("write off \n");
        return -1;
    }
    int mid = *m_pWr;

    this->noblock_set_data(mid, val);
    mid++;
    mid %= m_bufSize;
    *m_pWr = mid;
    count++;
    *m_pSize = count;
    m_sem.release(1);
    return 0;
}

template < class T >
int Sem_Share_Data< T >::read_send_data(T & val)
{
    ZLockerClass<Sem_Share_Data< T >> locker(this);
    locker.lock();
    int count = *m_pSize;
    if(count <= 0)
    {
        zprintf3("read send data error!\n");
        return -1;
    }
    int mid = *m_pRd;

    this->noblock_get_data(mid, val);

    mid++;
    mid %= m_bufSize;

    *m_pRd = mid;
    count--;
    *m_pSize = count;

    return 0;
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
        if(this->running)
        {
            this->running = 0;
            this->realeseSem();
            this->waitEnd();
        }
    }
    void run();
};

template < class T, class FAT >
void Sem_Pth_Data< T, FAT >::run(void)
{
    while (this->running)
    {
        if (this->wait_thread_sem() == 0)
        {
            T val;
            if (this->read_send_data(val) == 0)
            {
                if (this->z_callbak != NULL) //执行操作
                {
                    this->z_callbak(this->father, val);
                }
            }
        }
        else
        {
            zprintf1("Sem_Pth_Data error!\n");
            break;
        }
    }
}

#endif /*__SEMSHARE_H__*/
