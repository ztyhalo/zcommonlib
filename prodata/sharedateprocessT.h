#ifndef SHAREDATEPROCESST_H
#define SHAREDATEPROCESST_H

#include "zprint.h"

template < class T >
class ShareDataProcessT
{
public:
    T*      m_data;
    int     m_classSize;
public:
    ShareDataProcessT():m_data(NULL),m_classSize(0)
    {
        ;
    }
    virtual ~ShareDataProcessT()
    {
        zprintf3("ShareDataProcessT destruct!\n");
    }

    int set_data(int add, const T & val);
    int set_data(T* addr,  T  val);
    T    get_data(int add);
    T  *  getDataAddr(int add);
    T    get_data(const T* addr);
    int  get_data(int add, T& val);
    int  get_data(const T* addr, T& val);
    int  get_data(int addr, void * dest, int size);
    int  noblock_set_data(int add, T val);
    T    noblock_get_data(int add);
    int  noblock_get_data(int add, T& val);
    virtual bool dateLock(void) = 0;
    virtual bool  dateUnlock(void) = 0;
};



template < class T >
int ShareDataProcessT< T >::set_data(int add, const T & val)
{
    if (add >= m_classSize)
    {
        zprintf1("set data off\n");
        return -1;
    }
    dateLock();
    memcpy(this->m_data + add, &val, sizeof(T));
    dateUnlock();
    return 0;
}

template < class T >
int ShareDataProcessT< T >::set_data(T* addr,  T  val)
{
    if ((addr - this->m_data) >= m_classSize)
    {
        zprintf1("set data off\n");
        return -1;
    }
    dateLock();
    *addr = val;
    dateUnlock();
    return 0;
}

template < class T >
T ShareDataProcessT< T >::get_data(int add)
{
    if (add >= m_classSize)
    {
        zprintf3("get data off\n");
        return *this->m_data;
    }
    T mid;
    dateLock();
    mid = *(this->m_data + add);
    dateUnlock();
    return mid;
}

template < class T >
T* ShareDataProcessT< T >::getDataAddr(int add)
{
    dateLock();
    if (add >= m_classSize)
    {
        dateUnlock();
        zprintf3("get data off\n");
        return NULL;
    }
    dateUnlock();
    return this->m_data + add;
}

template < class T >
T ShareDataProcessT< T >::get_data(const T * addr)
{
    if ((addr - this->m_data) >= m_classSize)
    {
        zprintf3("get data off\n");
        return *this->m_data;
    }
    T mid;
    dateLock();
    mid = *(addr);
    dateUnlock();
    return mid;
}

template < class T >
int ShareDataProcessT< T >::get_data(int add, T & val)
{
    if (add >= m_classSize)
    {
        zprintf1("get data off\n");
        return -1;
    }

    dateLock();
    val = *(this->m_data + add);
    dateUnlock();
    return 0;
}
template < class T >
int ShareDataProcessT< T >::get_data(const T * addr, T & val)
{
    if ((addr - this->m_data) >= m_classSize)
    {
        zprintf1("get data off\n");
        return -1;
    }

    dateLock();
    val = *(addr);
    dateUnlock();
    return 0;
}

template < class T >
int ShareDataProcessT< T >::get_data(int addr, void * dest, int size)
{
    if (addr >= m_classSize || (addr +size) > m_classSize)
    {
        zprintf1("get data off\n");
        return -1;
    }
    if(dest == NULL)
    {
        zprintf1("ShareDataProcessT get data dest is NULL\n");
        return -2;
    }

    dateLock();
    memcpy(dest, this->m_data + addr, sizeof(T)*size);
    dateUnlock();
    return 0;
}

template < class T >
int ShareDataProcessT< T >::noblock_set_data(int add, T val)
{
    if (add >= m_classSize)
    {
        zprintf1("set data off\n");
        return -1;
    }
    memcpy(this->m_data + add, &val, sizeof(T));
    return 0;
}

template < class T >
T ShareDataProcessT< T >::noblock_get_data(int add)
{
    if (add >= m_classSize)
    {
        zprintf1("get data off\n");
        return *this->m_data;
    }
    T mid;
    mid = *(this->m_data + add);
    return mid;
}

template < class T >
int ShareDataProcessT< T >::noblock_get_data(int add, T & val)
{
    if (add >= m_classSize)
    {
        zprintf1("get data off\n");
        return -1;
    }
    val = *(this->m_data + add);
    return 0;
}

#endif // SHAREDATEPROCESST_H
