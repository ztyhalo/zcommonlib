#ifndef __BYTETURN_H__
#define __BYTETURN_H__
#include <byteswap.h>
#include <stdint.h>
#include <stdio.h>


#include <string>
#include <vector>
#include <assert.h>
#include <typeinfo>
#include <stdlib.h>
#include <cstring>
#include <stddef.h>
#include <QDomDocument>
#include "tbundle.h"

using namespace std;

template < typename Tbyteswap >

void tbyte_swap(Tbyteswap* turndata, uint8_t size)
{
    int        i;
    int        bytesize = 0;
    Tbyteswap* midvp    = NULL;

    bytesize = sizeof(Tbyteswap);
    if (size % bytesize)
        return;
    if (turndata == NULL)
        return;
    midvp = (Tbyteswap*) turndata;
    for (i = 0; i < size / bytesize; i++, midvp++)
    {
        if (bytesize == 4)
            *midvp = bswap_32((*midvp));
        else if (bytesize == 2)
            *midvp = bswap_16((*midvp));
    }
}

template < typename T >
void bit_opclear(T* turndata, uint8_t bit, uint8_t size)
{
    T mid     = (1 << size) - 1;
    mid       = ~(mid << bit);
    *turndata = (*turndata) & mid;
}

template < typename T >
void bit_opc_set(void* turndata, uint8_t bit, uint8_t size, T val)
{
    if (val >= (T) (1 << size))
        return;
    T* mid = (T*) turndata;
    bit_opclear(mid, bit, size);

    *mid = (*mid) | (val << bit);
}

#define ENUM_TYPE 0x01
#define BIT_TYPE  0x02
struct FieldMetadata
{
    // 成员变量的名称
    string m_name;

    // 成员变量的类型
    string m_type;

    // 成员变量的地址
    size_t m_offset;

    uchar m_stype; //子类型

    uchar m_bit;
    uchar m_bitSize;

    FieldMetadata(const string & name, const string & type, size_t offset, uchar stype = 0, uchar bit = 0, uchar btsize = 0):
        m_name(name),m_type(type),m_offset(offset),m_stype(stype),m_bit(bit),m_bitSize(btsize)
    {
        ;
    }
};

/*
 * 声明结构体类型
 */
#define Declare_Struct(class_type)                                                                                     \
  private:                                                                                                             \
    typedef class_type this_class;                                                                                     \
                                                                                                                       \
    template < int N, typename T = void >                                                                              \
    class Init_I                                                                                                       \
    {                                                                                                                  \
      public:                                                                                                          \
        explicit Init_I(vector< FieldMetadata >& metas)                                                                         \
        {                                                                                                              \
        }                                                                                                              \
    };                                                                                                                 \
    template < int N, typename T = void >                                                                              \
    class Init_Enum                                                                                                    \
    {                                                                                                                  \
      public:                                                                                                          \
        explicit Init_Enum(vector< enumpara >& metas)                                                                           \
        {                                                                                                              \
        }                                                                                                              \
    };

/*
 * 定义结构体变量
 */
#define Define_Field(var_index, var_type, var_name)                                                                    \
  public:                                                                                                              \
    var_type var_name;                                                                                                 \
                                                                                                                       \
  private:                                                                                                             \
    template < typename T >                                                                                            \
    class Init_I< var_index, T >                                                                                       \
    {                                                                                                                  \
      public:                                                                                                          \
        explicit Init_I(vector< FieldMetadata >& metas)                                                                         \
        {                                                                                                              \
            FieldMetadata fmd(#var_name, typeid(var_type).name(), offsetof(this_class, var_name));                     \
            metas.insert(metas.begin(), fmd);                                                                          \
        }                                                                                                              \
    };

struct enumpara
{
    uint   m_node;
    string m_name;
    uint   m_val;
    enumpara(uint node, const string & name, uint val):
        m_node(node),m_name(name),m_val(val)
    {
        ;
    }
};

#define Define_Enum(var_index, var_node, var_enum, var_value)                                                          \
    template < typename T >                                                                                            \
    class Init_Enum< var_index, T >                                                                                    \
    {                                                                                                                  \
      public:                                                                                                          \
        explicit Init_Enum(vector< enumpara >& metas)                                                                           \
        {                                                                                                              \
            enumpara mid(var_node, #var_enum, var_value);                                                              \
            metas.insert(metas.begin(), mid);                                                                          \
        }                                                                                                              \
    };

/*
 * 定义结构体变量
 */
#define Define_Style_SField(var_index, var_type, var_name, sz, bt, btsize)                                             \
  public:                                                                                                              \
    var_type var_name;                                                                                                 \
                                                                                                                       \
  private:                                                                                                             \
    template < typename T >                                                                                            \
    class Init_I< var_index, T >                                                                                       \
    {                                                                                                                  \
      public:                                                                                                          \
        explicit Init_I(vector< FieldMetadata >& metas)                                                                         \
        {                                                                                                              \
            FieldMetadata fmd(#var_name, typeid(var_type).name(), offsetof(this_class, var_name), sz, bt, btsize);     \
            metas.insert(metas.begin(), fmd);                                                                          \
        }                                                                                                              \
    };                                                                                                                 \
/*                                                                                                                     \
 * 定义结构体变量                                                                                               \
 */
#define Define_Style_Field(var_index, var_type, var_name, bit_name, sz, bt, btsize)                                    \
                                                                                                                       \
  private:                                                                                                             \
    template < typename T >                                                                                            \
    class Init_I< var_index, T >                                                                                       \
    {                                                                                                                  \
      public:                                                                                                          \
        explicit Init_I(vector< FieldMetadata >& metas)                                                                         \
        {                                                                                                              \
            FieldMetadata fmd(#var_name, typeid(var_type).name(), offsetof(this_class, bit_name), sz, bt, btsize);     \
            metas.insert(metas.begin(), fmd);                                                                          \
        }                                                                                                              \
    };

#define Define_Bit_SField(var_index, var_type, var_name, bt, btsize)                                                   \
    Define_Style_SField(var_index, var_type, var_name, BIT_TYPE, bt, btsize)

#define Define_Bit_Field(var_index, var_type, var_name, bit_name, bt, btsize)                                          \
    Define_Style_Field(var_index, var_type, var_name, bit_name, BIT_TYPE, bt, btsize)

#define Define_Enum_Field(var_index, var_type, var_name)                                                               \
    Define_Style_SField(var_index, var_type, var_name, ENUM_TYPE, 0, 0)
#define Define_EnBit_SField(var_index, var_type, var_name, bt, btsize)                                                 \
    Define_Style_SField(var_index, var_type, var_name, ENUM_TYPE | BIT_TYPE, bt, btsize)
#define Define_EnBit_Field(var_index, var_type, var_name, bit_name, bt, btsize)                                        \
    Define_Style_Field(var_index, var_type, var_name, bit_name, ENUM_TYPE | BIT_TYPE, bt, btsize)
/*
 * 定义结构体元数据
 */
#define Define_Metadata(var_count)                                                                                     \
  public:                                                                                                              \
    static vector< FieldMetadata > fieldinfo;                                                                          \
                                                                                                                       \
  private:                                                                                                             \
    template < int N, typename T = void >                                                                              \
    class CallInits                                                                                                    \
    {                                                                                                                  \
      public:                                                                                                          \
        explicit CallInits(vector< FieldMetadata >& metas)                                                                      \
        {                                                                                                              \
            Init_I< N >        in(metas);                                                                              \
            CallInits< N - 1 > ci(metas);                                                                              \
        }                                                                                                              \
    };                                                                                                                 \
                                                                                                                       \
    template < typename T >                                                                                            \
    class CallInits< 1, T >                                                                                            \
    {                                                                                                                  \
      public:                                                                                                          \
        explicit CallInits(vector< FieldMetadata >& metas)                                                                      \
        {                                                                                                              \
            Init_I< 1 > in(metas);                                                                                     \
        }                                                                                                              \
    };                                                                                                                 \
                                                                                                                       \
    static vector< FieldMetadata > Init()                                                                              \
    {                                                                                                                  \
        vector< FieldMetadata > fmd;                                                                                   \
        CallInits< var_count >  ci(fmd);                                                                               \
        return fmd;                                                                                                    \
    }

#define Define_Meta_Enumdata(var_count)                                                                                \
  public:                                                                                                              \
    static vector< enumpara > enuminfo;                                                                                \
                                                                                                                       \
  private:                                                                                                             \
    template < int N, typename T = void >                                                                              \
    class EnumInits                                                                                                    \
    {                                                                                                                  \
      public:                                                                                                          \
        explicit EnumInits(vector< enumpara >& metas)                                                                           \
        {                                                                                                              \
            Init_Enum< N >     in(metas);                                                                              \
            EnumInits< N - 1 > ci(metas);                                                                              \
        }                                                                                                              \
    };                                                                                                                 \
                                                                                                                       \
    template < typename T >                                                                                            \
    class EnumInits< 1, T >                                                                                            \
    {                                                                                                                  \
      public:                                                                                                          \
        explicit EnumInits(vector< enumpara >& metas)                                                                           \
        {                                                                                                              \
            Init_Enum< 1 > in(metas);                                                                                  \
        }                                                                                                              \
    };                                                                                                                 \
                                                                                                                       \
    static vector< enumpara > EInit()                                                                                  \
    {                                                                                                                  \
        vector< enumpara >     fmd;                                                                                    \
        EnumInits< var_count > ci(fmd);                                                                                \
        return fmd;                                                                                                    \
    }

#define Define_NoMeta_Enumdata                                                                                         \
  public:                                                                                                              \
    static vector< enumpara > enuminfo;                                                                                \
    static vector< enumpara > EInit()                                                                                  \
    {                                                                                                                  \
        vector< enumpara > fmd;                                                                                        \
        return fmd;                                                                                                    \
    }

/*
 * 实现结构体类型
 */
#define Implement_Struct(class_type) vector< FieldMetadata > class_type::fieldinfo = class_type::Init();

#define Implement_Struct_Enum(class_type) vector< enumpara > class_type::enuminfo = class_type::EInit();

typedef unsigned char byte;

/*
 * 解析字符串到类型
 */
template < typename _T, typename ST = char >
class ParaseToType
{
  private:
    /*
     * 内存值拷贝
     */
    void CopyValueOnMemory(byte* memvalue, const string & type, const string & strvalue)
    {
        int    off   = strvalue.find('=');
        string name  = strvalue.substr(0, off);
        string value = strvalue.substr(off + 1);

        if (type.compare(typeid(int).name()) == 0)
        {
            int vl = atoi(value.c_str());
            memcpy(memvalue, &vl, sizeof(int));
        }
        else if (type.compare(typeid(double).name()) == 0)
        {
            double vl = atof(value.c_str());
            memcpy(memvalue, &vl, sizeof(double));
        }
        else if (type.compare(typeid(string).name()) == 0)
        {
            string* strmem = (string*) memvalue;
            strmem->append(value);
        }
        else
        {
            /*
             * Only support the following types
             * int, double, std::string
             */
            assert(false);
        }
    }

  public:
    /*
     * 字符串的格式:"a=5;b=6.0f;c=766666666hhhhhgfdd"
     */
    bool Parase(_T& t, const string & strvalue)
    {
        int stroffset = 0;

        for (auto iter = _T::fieldinfo.begin(); iter != _T::fieldinfo.end(); iter++)
        {
            int    newoffset = strvalue.find(';', stroffset);
            string ty        = strvalue.substr(stroffset, newoffset - stroffset);

            byte* th = (((byte*) &t) + (*iter).m_offset);
            CopyValueOnMemory(th, (*iter).m_type, ty);

            stroffset = newoffset + 1;
        }

        return true;
    }
    bool set_val(_T& t, void* add)
    {

        for (auto iter = _T::fieldinfo.begin(); iter != _T::fieldinfo.end(); iter++)
        {

            if ((*iter).m_type.compare(typeid(int).name()) == 0)
            {
                byte* th = (((byte*) &t) + (*iter).m_offset);
                memcpy((byte*) add + (*iter).m_offset, th, sizeof(int));
            }
            else if ((*iter).m_type.compare(typeid(double).name()) == 0)
            {

                byte* th = (((byte*) &t) + (*iter).m_offset);
                memcpy((byte*) add + (*iter).m_offset, th, sizeof(double));
            }
            else if ((*iter).m_type.compare(typeid(string).name()) == 0)
            {
                byte*   th     = (((byte*) add) + (*iter).m_offset);
                string* strmem = (string*) th;

                string* value = (string*) (((byte*) &t) + (*iter).m_offset);

                strmem->append(value->c_str());
            }
            else
            {
                /*
                 * Only support the following types
                 * int, double, std::string
                 */
                assert(false);
            }
        }

        return true;
    }
    bool get_val(void* add, _T& t)
    {

        for (auto iter = _T::fieldinfo.begin(); iter != _T::fieldinfo.end(); iter++)
        {

            if ((*iter).m_type.compare(typeid(int).name()) == 0)
            {
                byte* th = (((byte*) &t) + (*iter).m_offset);
                memcpy(th, (byte*) add + (*iter).m_offset, sizeof(int));
            }
            else if ((*iter).m_type.compare(typeid(double).name()) == 0)
            {

                byte* th = (((byte*) &t) + (*iter).m_offset);
                memcpy(th, (byte*) add + (*iter).m_offset, sizeof(double));
            }
            else if ((*iter).m_type.compare(typeid(string).name()) == 0)
            {
                byte*   th     = (((byte*) &t) + (*iter).m_offset);
                string* strmem = (string*) th;

                string* value = (string*) (((byte*) add) + (*iter).m_offset);
                strmem->append(value->c_str());
            }
            else
            {
                /*
                 * Only support the following types
                 * int, double, std::string
                 */
                assert(false);
            }
        }

        return true;
    }
    template < typename T >
    int string_to_data(const FieldMetadata & m, const QString & val, T * add)
    {
        int err = 0;
        T   va  = 0;
        if ((m.m_stype & 0x01) == 0)
        {
            if (m.m_type.compare(typeid(int).name()) == 0)
                va = val.toInt();
            else if (m.m_type.compare(typeid(short).name()) == 0)
            {
                va = val.toShort();
            }
            else if (m.m_type.compare(typeid(int8_t).name()) == 0)
            {
                va = (int8_t) val.toShort();
            }
            else if (m.m_type.compare(typeid(uint).name()) == 0)
            {
                va = val.toUInt();
            }
            else if (m.m_type.compare(typeid(ushort).name()) == 0)
            {
                va = val.toUShort();
            }
            else if (m.m_type.compare(typeid(char).name()) == 0)
            {
                QByteArray ba = val.toLatin1();
                char*      mm = ba.data();
                //             const char * midch = val.toStdString().data();
                va = mm[0];
            }
            else if (m.m_type.compare(typeid(uchar).name()) == 0)
            {
                va = (uchar) val.toUShort();
            }
            else if (m.m_type.compare(typeid(double).name()) == 0)
            {
                va = val.toDouble();
            }
            else
            {
                err = -1;
                return err;
            }
        }
        else
        {
            uint midv = 0;
            get_enum_data(val.toStdString(), midv);
            va = midv;
        }

        if (m.m_stype & 0x02)
        {
            bit_opc_set(add, m.m_bit, m.m_bitSize, (T) va);
        }
        else
            memcpy(add, &va, sizeof(T));
        return err;
    }

    int string_to_double(const FieldMetadata & m, const QString & val, double* add)
    {
        int    err = 0;
        double va  = 0;
        if ((m.m_stype & 0x01) == 0)
        {
            va = val.toDouble();
        }
        else
        {
            uint midv = 0;
            get_enum_data(val.toStdString(), midv);
            va = midv;
        }

        if (m.m_stype & 0x02)
        {
            return -2;
        }
        else
            memcpy(add, &va, sizeof(double));
        return err;
    }
    int string_data(const FieldMetadata & m, const QString & val, void* add)
    {
        int err = 0;
        if (m.m_type.compare(typeid(int).name()) == 0)
        {
            string_to_data(m, val, (int*) add);
        }
        else if (m.m_type.compare(typeid(short).name()) == 0)
        {
            string_to_data(m, val, (short*) add);
        }
        else if (m.m_type.compare(typeid(int8_t).name()) == 0)
        {
            string_to_data(m, val, (int8_t*) add);
        }
        else if (m.m_type.compare(typeid(uint).name()) == 0)
        {
            string_to_data(m, val, (uint*) add);
        }
        else if (m.m_type.compare(typeid(ushort).name()) == 0)
        {
            string_to_data(m, val, (ushort*) add);
        }
        else if (m.m_type.compare(typeid(uchar).name()) == 0)
        {
            string_to_data(m, val, (uchar*) add);
        }
        else if (m.m_type.compare(typeid(char).name()) == 0)
        {
            string_to_data(m, val, (char*) add);
        }
        else if (m.m_type.compare(typeid(double).name()) == 0)
        {
            string_to_double(m, val, (double*) add);
        }
        else if (m.m_type.compare(typeid(string).name()) == 0)
        {
            string* strmem = (string*) add;
            strmem->clear();
            strmem->append(val.toStdString());
        }
        else
        {
            err = -1;
        }
        return err;
    }

    int get_enum_data(const string & st, uint& val)
    {
        for (auto iter = _T::enuminfo.begin(); iter != _T::enuminfo.end(); iter++)
        {
            if (iter->m_name == st)
            {
                val = iter->m_val;
                return 0;
            }
        }
        return -1;
    }
    int get_enum_data(uint id, const string & st, uint& val)
    {
        for (auto iter = _T::enuminfo.begin(); iter != _T::enuminfo.end(); iter++)
        {
            if (iter->m_node == id && iter->m_name == st)
            {
                val = iter->m_val;
                return 0;
            }
        }
        return -1;
    }

    int get_enum_string(uint id, vector< string >& str)
    {
        int size = 0;
        for (auto iter = _T::enuminfo.begin(); iter != _T::enuminfo.end(); iter++)
        {
            if (iter->m_node == id)
            {
                str.push_back(iter->m_name);
                size++;
            }
        }
        return size;
    }

    bool read_xml_data(_T & t, const string & leaguer, const QString & val)
    {
        for (auto iter = _T::fieldinfo.begin(); iter != _T::fieldinfo.end(); iter++)
        {
            if (iter->m_name == leaguer)
            {
                void* th = (((byte*) &t) + (*iter).m_offset);

                string_data(*iter, val, th);
            }
        }
        return TRUE;
    }
    bool read_xml_data(void* t, const string & leaguer, const QString & val)
    {
        for (auto iter = _T::fieldinfo.begin(); iter != _T::fieldinfo.end(); iter++)
        {
            if (iter->m_name == leaguer)
            {
                void* th = (((byte*) t) + (*iter).m_offset);

                string_data(*iter, val, th);
            }
        }
        return TRUE;
    }

    bool read_xml_data(_T& t, const QDomElement & val)
    {
        for (auto iter = _T::fieldinfo.begin(); iter != _T::fieldinfo.end(); iter++)
        {
            QString va = val.attribute(QString::fromStdString(iter->m_name));

            void* th = (((byte*) &t) + (*iter).m_offset);
            string_data(*iter, va, th);
        }
        return TRUE;
    }
    bool read_xml_data(const QDomElement & val, void* add, uint size)
    {
        _T t;
        for (auto iter = _T::fieldinfo.begin(); iter != _T::fieldinfo.end(); iter++)
        {
            QString va = val.attribute(QString::fromStdString(iter->m_name));

            void* th = (((byte*) &t) + (*iter).m_offset);
            string_data(*iter, va, th);
        }
        memcpy(add, &t, size);
        return true;
    }
    bool read_xml_data(void* add, const QDomElement & val)
    {
        for (auto iter = _T::fieldinfo.begin(); iter != _T::fieldinfo.end(); iter++)
        {
            QString va = val.attribute(QString::fromStdString(iter->m_name));

            void* th = (((byte*) add) + (*iter).m_offset);
            string_data(*iter, va, th);
        }
        return true;
    }

    bool set_bundle(void* add, TBundle bundle)
    {
        for (auto iter = _T::fieldinfo.begin(); iter != _T::fieldinfo.end(); iter++)
        {
            QString key = QString::fromStdString(iter->m_name);
            if (bundle.contains(key))
            {
                QString va = bundle.get(key).toString();
                void*   th = (((byte*) add) + (*iter).m_offset);
                string_data(*iter, va, th);
            }
        }
        return true;
    }
};

struct Test
{
    Declare_Struct(Test)

        Define_Field(1, int, a)

            Define_Field(2, double, b)

                Define_Field(3, string, c)

                    Define_Enum_Field(4, int, d)

                        Define_Metadata(4)

                            Define_Enum(1, 3, open, 0) Define_Enum(2, 3, out, 1) Define_Enum(3, 3, in, 2)
                                Define_Enum(4, 3, freq, 3) Define_Enum(5, 3, state, 4) Define_Enum(6, 4, yes, 1)
                                    Define_Enum(7, 4, no, 0)

                                        Define_Meta_Enumdata(7)
};

void byte16_swap(void* turndata, uint8_t size);

#endif //__BYTETURN_H__
