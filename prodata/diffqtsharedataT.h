#ifndef DIFFQTSHAREDATAT_H
#define DIFFQTSHAREDATAT_H
#include "qtsharemem.h"

template < class T >
class DiffQTShareDataT : public QTShareDataT< T >
{

  public:
    T*      m_pRead;
    int     m_tSize;
    int     m_unitsize;  // 数据单元长度，用于计算dirty标记区域大小
    int     m_exsize;    // 总体大小，数据区+dirty标记区
    char*   m_dirtyData; // dirty标记区
    int     m_dirtySize; // dirty标记区字节数
    bool    m_isDirtyEn;

  public:
    DiffQTShareDataT():m_pRead(NULL),m_tSize(0),m_unitsize(0),m_exsize(0),m_dirtyData(NULL),
          m_dirtySize(0),m_isDirtyEn(false)
    {
        ;
    }
    virtual ~DiffQTShareDataT()
    {
        zprintf3("DiffQTShareDataT destruct\n");
    }
    // int  create_data(int size);
    int  creat_data(int size, const QString & keyid, int unitsize);

    void set_data(int add, const T & val);
    // void set_data(T* add, T val);
    // T    get_data(uint add);
    // int  get_data(uint add, T& val);
    int  destroy();

    // void shm_lock();
    // void shm_unlock();
    int  getDiffData(void* out, int* len);
    int  setDiffData(void** in, int len, QList< int >& addrList);
    int  clearDirtyData();
    void setDirtyEn(bool enable);
};

template < class T >
int DiffQTShareDataT< T >::destroy()
{
    return 0;
}

union DirtyFlag
{
    uchar flag;
    struct bitfield
    {
        char bit0 : 1;
        char bit1 : 1;
        char bit2 : 1;
        char bit3 : 1;
        char bit4 : 1;
        char bit5 : 1;
        char bit6 : 1;
        char bit7 : 1;
    } bits;
};

inline void setDirty(DirtyFlag* dirty, int bitindex)
{
    switch (bitindex)
    {
        case 0:
            dirty->bits.bit0 = 1;
            break;
        case 1:
            dirty->bits.bit1 = 1;
            break;
        case 2:
            dirty->bits.bit2 = 1;
            break;
        case 3:
            dirty->bits.bit3 = 1;
            break;
        case 4:
            dirty->bits.bit4 = 1;
            break;
        case 5:
            dirty->bits.bit5 = 1;
            break;
        case 6:
            dirty->bits.bit6 = 1;
            break;
        case 7:
            dirty->bits.bit7 = 1;
            break;
        default:
            break;
    }
}


template < class T >
int DiffQTShareDataT< T >::clearDirtyData()
{
    if (!m_isDirtyEn)
        return 0;
    this->lock();
    void* addr = m_dirtyData;
    memset(addr, 0, m_dirtySize);
    this->unlock();

    return 0;
}

template < class T >
void DiffQTShareDataT< T >::setDirtyEn(bool enable)
{
    m_isDirtyEn = enable;
}


template < class T >
int DiffQTShareDataT< T >::creat_data(int size, const QString & keyid, int unitsize)
{
    // shm_key       = keyid;
    m_unitsize    = unitsize;
    int unitcount = size / unitsize;
    // unitcount为8的倍数，不满足补齐
    if (unitcount % 8 != 0)
    {
        unitcount = unitcount / 8 + 1;
    }
    else
    {
        unitcount = unitcount / 8;
    }
    m_tSize = size/sizeof(T);
    m_exsize    = size + unitcount;
    m_dirtySize = unitcount;
    return this->create_data(m_exsize, keyid);
}

template < class T >
void DiffQTShareDataT< T >::set_data(int add, const T & val)
{
    if (add >= m_tSize)
    {
        zprintf1("DiffQTShareDataT set data off\n");
        return;
    }

    this->lock();
    memcpy(this->m_data + add, &val, sizeof(T));

    if (m_isDirtyEn)
    {
        // 写dirty标记
        uint l_add     = add / m_unitsize;
        int  byteindex = l_add >> 3; // 右移3位，代替除以8
        int  bitindex  = l_add - byteindex * 8;

        DirtyFlag* dirty = (DirtyFlag*)(this->m_dirtyData + byteindex);
        setDirty(dirty, 7 - bitindex);
    }
    this->unlock();
}


// 获取差异数据
template < class T >
int DiffQTShareDataT< T >::getDiffData(void* out, int* len)
{
    if (!m_isDirtyEn)
        return -1;
    // copy dirty标记区
    char* l_dirtydata = new char[m_dirtySize];
    this->lock();
    memcpy(l_dirtydata, m_dirtyData, m_dirtySize);
    memset(m_dirtyData, 0, m_dirtySize);
    this->unlock();

    char* pData   = l_dirtydata;
    char* pOut    = static_cast<char *>(out);
    char  l_val   = 0;
    // char  l_data1 = 0;
    // char  l_data2 = 0;
    int   i, j;
    int   l_total = 0;

    for (i = 0; i < m_dirtySize; i++) // 遍历dirty标记区
    {
        l_val = *pData;
        if (l_val != 0)
        {
            for (j = 0; j < 8; j++)
            {
                if ((l_val >> (7 - j)) & 1)
                {
                    l_total++;
                    // output data
                    uint l_addr = i * 8 + j;
                    memcpy(pOut, &l_addr, 4);
                    pOut    = pOut + 4;
                    // l_data1 = *(this->m_data + (l_addr * m_unitsize));
                    // l_data2 = *(this->m_data + (l_addr * m_unitsize) + 1);
                    memcpy(pOut, this->m_data + (l_addr * m_unitsize), sizeof(T) * m_unitsize);
                    pOut = pOut + sizeof(T) * m_unitsize;
                    // qDebug() << "[QTShareDataT] getDiffData l_addr:" << l_addr << ", l_data:" << (quint8) l_data1 << ","
                    //          << (quint8) l_data2;
                }
            }
        }
        pData++;
    }

    *len = l_total * (sizeof(T) * m_unitsize + 4);

    delete[] l_dirtydata;
    return l_total;
}
template < class T >
int DiffQTShareDataT< T >::setDiffData(void** in, int len, QList< int >& addrList)
{
    addrList.clear();
    if (len <= 0)
        return -1;

    this->lock();
    int   l_total = len / (sizeof(T) * m_unitsize + 4);
    char* pIn     = (char*) (*in);
    // char  l_data1 = 0;
    // char  l_data2 = 0;

    for (int i = 0; i < l_total; i++)
    {
        uint l_addr;
        memcpy(&l_addr, pIn, 4);
        pIn = pIn + 4;
        memcpy(this->m_data + (l_addr * m_unitsize), pIn, sizeof(T) * m_unitsize);
        pIn     = pIn + +sizeof(T) * m_unitsize;
        // l_data1 = *(this->data + (l_addr * m_unitsize));
        // l_data2 = *(this->data + (l_addr * m_unitsize) + 1);

        addrList.append(l_addr);
    }
    this->unlock();
    return l_total;
}

#endif // DIFFQTSHAREDATAT_H
