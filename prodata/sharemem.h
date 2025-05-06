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


#include <string.h>
#include "zsysshm.h"


using namespace std;


// linux共享内存 不继承线程类
template < class T >
class ShareDataT : public ZSysShm
{
public:

    T*      m_pType;
private:
  int       m_classSize;
public:
    ShareDataT():m_pType(NULL),m_classSize(0)
    {
        ;
    }
    virtual ~ShareDataT()
    {
        zprintf3("ShareDataT destruct\n");
    }

    int creat_data(int size, key_t id);
    int read_creat_data(key_t id, int size = 0);
    void set_data(int add, const T & val);
    T    get_data(int add);
    int  get_data(int add, T & val);
};


// template < class T >
// int ShareDataT< T >::creat_data(int size)
// {
//     zprintf2("shm id is %d\n", m_shmKey);

//     if(!init_key())
//     {
//         zprintf1("sharedatat creat data init key err!\n");
//         return -1;
//     }
//     m_sysSem.setKey(std::to_string(m_shmKey), 1, ZSystemSem::Create);

//     ZLockerClass<ShareDataT<T>> locker(this);
//     locker.lock();

//     if(!create(size))
//         return -2;
//     this->m_size = size;

//     if(!attach())
//     {
//         zprintf1("ShareDataT attach fail!\n");
//         return -3;
//     }

//     this->m_data = (T*) m_memory;

//     return 0;
// }

template < class T >
int ShareDataT< T >::creat_data(int size, key_t id)
{
    m_shmKey = id;
    if(createData(size) == 0)
    {
        m_pType = (T*) m_memory;
        m_classSize = (int)(this->m_size / sizeof(T));
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
        m_pType = (T*) m_memory;
        m_classSize = (int)(this->m_size / sizeof(T));
        return 0;
    }
    zprintf1("ShareDataT read create keyid %d error!\n", id);
    return -1;
}


template < class T >
void ShareDataT< T >::set_data(int add, const T & val)
{
    if (add >= m_classSize)
    {
        zprintf1("ShareDataT set data off\n");
        return;
    }
    memcpy(m_pType + add, &val, sizeof(T));
}

template < class T >
T ShareDataT< T >::get_data(int add)
{
    if (add >= m_classSize)
    {
        zprintf3("ShareDataT get data off\n");
        return *m_pType;
    }
    return *(m_pType + add);
}

template < class T >
int ShareDataT< T >::get_data(int add, T & val)
{
    if (add >= m_classSize)
    {
        zprintf3("ShareDataT get data off\n");
        return -1;
    }
    val = *(m_pType + add);
    return 0;
}

#endif /*__SHAREMEM_H__*/
