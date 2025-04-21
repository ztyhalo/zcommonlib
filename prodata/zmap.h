/****************************************************
 *************zmap库**********************
 *Version: 1.0
 *History: 2017.7.7
 *
 ****************************************************/

#ifndef __ZMAP_H__
#define __ZMAP_H__
#include <sys/types.h>
#include <map>
#include <string.h>
#include "zprint.h"

using namespace std;

template < class KEYTY, class DTYPE >
class Z_Map
{
  public:
    map< KEYTY, DTYPE > zmap;

  public:
    Z_Map()
    {
        ;
    }
    virtual ~Z_Map()
    {
        zprintf3("z_map distruct!\n");
    }
    DTYPE  get_data(int num);
    DTYPE* get_datap(int num);
    bool   is_have(KEYTY k);
};

template < class KEYTY, class DTYPE >
DTYPE Z_Map< KEYTY, DTYPE >::get_data(int num)
{
    int   i = 0;
    DTYPE ret;
    memset(&ret, 0x00, sizeof(DTYPE));

    typename map< KEYTY, DTYPE >::iterator iter;
    for (iter = zmap.begin(); iter != zmap.end(); ++iter, ++i)
    {
        if (num == i)
            return iter->second;
    }
    return ret;
}

template < class KEYTY, class DTYPE >
DTYPE* Z_Map< KEYTY, DTYPE >::get_datap(int num)
{
    int i = 0;

    typename map< KEYTY, DTYPE >::iterator iter;
    for (iter = zmap.begin(); iter != zmap.end(); ++iter, ++i)
    {
        if (num == i)
            return &(iter->second);
    }
    return NULL;
}
template < class KEYTY, class DTYPE >
bool Z_Map< KEYTY, DTYPE >::is_have(KEYTY k)
{
    return (zmap.find(k) != zmap.end());
}

template < class KEYTY, class DTYPE >
class Zt_Map : public map< KEYTY, DTYPE >
{
  public:
    typedef struct
    {
        KEYTY key;
        DTYPE data;
    } KEY_DATA;

  public:
    Zt_Map()
    {
        ;
    }
    virtual ~Zt_Map()
    {
        zprintf3("Zt_Map distruct!\n");
    }
    DTYPE& val(KEYTY key);
    DTYPE  get_order_data(int num);
    DTYPE* get_order_datap(int num);
    bool   is_have(KEYTY k);
    void   many_insert(KEY_DATA* data, int size);
};

template < class KEYTY, class DTYPE >
DTYPE Zt_Map< KEYTY, DTYPE >::get_order_data(int num)
{
    int   i = 0;
    DTYPE ret;
    memset(&ret, 0x00, sizeof(DTYPE));

    typename map< KEYTY, DTYPE >::iterator iter;
    for (iter = this->begin(); iter != this->end(); ++iter, ++i)
    {
        if (num == i)
            return iter->second;
    }
    return ret;
}

template < class KEYTY, class DTYPE >
DTYPE& Zt_Map< KEYTY, DTYPE >::val(KEYTY key)
{
    typename map< KEYTY, DTYPE >::iterator iter;
    iter = this->find(key);
    return iter->second;
}

template < class KEYTY, class DTYPE >
DTYPE* Zt_Map< KEYTY, DTYPE >::get_order_datap(int num)
{
    int i = 0;

    typename map< KEYTY, DTYPE >::iterator iter;
    for (iter = this->begin(); iter != this->end(); ++iter, ++i)
    {
        if (num == i)
            return &(iter->second);
    }
    return NULL;
}
template < class KEYTY, class DTYPE >
bool Zt_Map< KEYTY, DTYPE >::is_have(KEYTY k)
{
    return (this->find(k) != this->end());
}
template < class KEYTY, class DTYPE >
void Zt_Map< KEYTY, DTYPE >::many_insert(KEY_DATA* d, int size)
{
    for (int i = 0; i < size; ++i)
    {
        this->insert(pair< KEYTY, DTYPE >(d->key, d->data));
        d++;
    }
}

#define MAP_IS_HAVE(map, key) ((map).find(key) != (map).end())

#endif /*__ZMAP_H__*/
