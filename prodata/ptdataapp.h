/****************************************************
 *************进程间通讯数据库**********************
 *Version: 1.1
 *History: 2017.7.3
 *         2017.7.6添加回调类
 *         2017.7.11 将1个文件拆分为多个文件 此文件为针对pt的数据组装
 ****************************************************/
#ifndef __PTDATAAPP_H__
#define __PTDATAAPP_H__

#include <map>
#include "semshare.h"
#include "qtsemshare.h"

//多个设备共用一个共享空间 PTYPE共享内存类型 DTYPE数据类型
template < class PTYPE, class DTYPE >
class DATAS_T
{
  public:
    PTYPE                           pdata;
    map< uint, creatdata< DTYPE > > devrdinfo;

  public:
    DATAS_T()
    {
        ;
    }
    virtual ~DATAS_T()
    {
        zprintf3("destory DATAS_T!\n");
    }

    int add_dev(uint devid, creatdata< DTYPE > in);
    int add_node_dev(uint devid, uint node);
    int set_dev_invalue(uint devid, uint in, DTYPE val);
    int get_dev_invalue(uint devid, uint in, DTYPE& val);
};

template < class PTYPE, class DTYPE >
int DATAS_T< PTYPE, DTYPE >::add_dev(uint devid, creatdata< DTYPE > in)
{
    devrdinfo.insert(pair< uint, creatdata< DTYPE > >(devid, in));
    return 1;
}

template < class PTYPE, class DTYPE >
int DATAS_T< PTYPE, DTYPE >::set_dev_invalue(uint devid, uint in, DTYPE val)
{
    typename map< uint, creatdata< DTYPE > >::iterator proiter;

    proiter = devrdinfo.find(devid);

    if (proiter != devrdinfo.end())
    {
        pdata.set_data((proiter->second.data - pdata.data) + in, val);
        return 0;
    }
    zprintf1("set_dev_invalue set devid %d in %d fail!\n", devid, in);
    return -1;
}

template < class PTYPE, class DTYPE >
int DATAS_T< PTYPE, DTYPE >::get_dev_invalue(uint devid, uint in, DTYPE& val)
{
    typename map< uint, creatdata< DTYPE > >::iterator proiter;

    proiter = devrdinfo.find(devid);

    if (proiter != devrdinfo.end())
    {
        return pdata.get_data((proiter->second.data - pdata.data) + in, val);
    }
    zprintf1("get_dev_invalue set devid %d in %d fail!\n", devid, in);
    return -1;
}

template < class PTYPE, class DTYPE >
int DATAS_T< PTYPE, DTYPE >::add_node_dev(uint devid, uint node)
{
    creatdata< DTYPE > mid;

    mid.data_init(pdata.addp, node * sizeof(DTYPE));

    pdata.addp += node;

    return add_dev(devid, mid);
}
template < class DTYPE >
class DATAS_Map_T
{
  public:
    DTYPE*                          mapdata_p;
    map< uint, creatdata< DTYPE > > devrdinfo;

  public:
    DATAS_Map_T():mapdata_p(0)
    {
        ;
    }
    virtual ~DATAS_Map_T()
    {
        zprintf3("DATAS_Map_T destruct!\n");
    }

    int    add_dev(uint devid, creatdata< DTYPE > in);
    int    add_node_dev(uint devid, uint node);
    DTYPE* get_dev_node_addr(uint devid, uint in);
};

template < class DTYPE >
int DATAS_Map_T< DTYPE >::add_dev(uint devid, creatdata< DTYPE > in)
{
    devrdinfo.insert(pair< uint, creatdata< DTYPE > >(devid, in));
    return 1;
}

template < class DTYPE >
DTYPE* DATAS_Map_T< DTYPE >::get_dev_node_addr(uint devid, uint in)
{
    typename map< uint, creatdata< DTYPE > >::iterator proiter;

    proiter = devrdinfo.find(devid);

    if (proiter != devrdinfo.end())
    {
        return (proiter->second.data + in);
    }
    zprintf1("get_dev_node_addr devid %d in %d fail!\n", devid, in);
    return NULL;
}

template < class DTYPE >
int DATAS_Map_T< DTYPE >::add_node_dev(uint devid, uint node)
{
    creatdata< DTYPE > mid(mapdata_p, node * sizeof(DTYPE));
    mapdata_p += node;
    return add_dev(devid, mid);
}
// qt类型的多个设备共享空间
template < class DTYPE >
class QTS_DATAS_T : public DATAS_T< QTShareDataT< DTYPE >, DTYPE >
{
public:
    QTS_DATAS_T()
    {
        ;
    }
    virtual ~QTS_DATAS_T()
    {
        zprintf3("QTS_DATAS_T destruct!\n");
    }
};

// linux共享内存加信号类型的多个设备数据组合
template < class DTYPE, class FAT >
class SemS_DATAS_T : public DATAS_T< Sem_Pth_Data< DTYPE, FAT >, DTYPE >
{
public:
    SemS_DATAS_T()
    {
        ;
    }
    virtual ~SemS_DATAS_T()
    {
        zprintf3("SemS_DATAS_T destruct!\n");
    }

};

// qt共享内存加linux信号类型的多个设备数据组合
template < class DTYPE, class FAT >
class SemS_QtDATAS_T : public DATAS_T< Sem_QtPth_Data< DTYPE, FAT >, DTYPE >
{
  public:
    SemS_QtDATAS_T()
    {
        ;
    }
    ~SemS_QtDATAS_T()
    {
        zprintf3("destory SemS_QtDATAS_T!\n");
    }
};

#endif /*__PTDATAAPP_H__*/
