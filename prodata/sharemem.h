/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2017.7.11 将1个文件拆分为多个文件
 *         2025.4.18 优化
 *
 ****************************************************/

#ifndef __SHAREMEM_H__
#define __SHAREMEM_H__


// #include <string.h>
#include "zsysshm.h"
#include "sharedateprocessT.h"

using namespace std;


// linux共享内存 不继承线程类
template < class T >
class ShareDataT : public ZSysShm, public ShareDataProcessT<T>
{
public:
    ShareDataT()
    {
        ;
    }
    virtual ~ShareDataT()
    {
        zprintf3("ShareDataT destruct\n");
    }

    int shareCreateData(int size, key_t id, AccessMode mode);
    int create_data(int size, key_t id);
    int read_creat_data(key_t id, int size = 0);

    bool dateLock(void) override;
    bool dateUnlock(void) override;
};


template < class T >
int ShareDataT< T >::create_data(int size, key_t id)
{
    m_shmKey = id;
    if(createData(size) == 0)
    {
        this->m_data = (T*) m_memory;
        this->m_classSize = (int)(this->m_size / sizeof(T));
        return 0;
    }
    zprintf1("ShareDataT create keyid %d error!\n", id);
    return -1;

}

template < class T >
int ShareDataT< T >::read_creat_data(key_t id, int size)
{

    if(readCreateData(id, size) == 0)
    {
        this->m_data = (T*) m_memory;
        this->m_classSize = (int)(this->m_size / sizeof(T));
        return 0;
    }
    zprintf1("ShareDataT read create keyid %d error!\n", id);
    return -1;
}

template < class T >
int ShareDataT< T >::shareCreateData(int size, key_t id, AccessMode mode)
{
    if(mode == Open)
    {
        return read_creat_data(id, size);
    }
    else
        return create_data(size, id);
}

template < class T >
bool ShareDataT< T >::dateLock(void)
{
       return this->lock();
}

template < class T >
bool ShareDataT< T >::dateUnlock(void)
{
    return this->unlock();
}

#endif /*__SHAREMEM_H__*/
