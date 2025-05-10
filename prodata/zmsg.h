﻿/****************************************************
 *************zmap库**********************
 *Version: 1.0
 *History: 2017.7.7
 *
 ****************************************************/

#ifndef __ZMSG_H__
#define __ZMSG_H__
#include <sys/types.h>

#include <stdio.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/msg.h>
#include "zprint.h"

using namespace std;

template < class MSGDATA >
class Z_Msg
{
  private:
    int m_msgKey;
    int m_msgId;
    int m_msgType;
    int m_created;

  public:
    typedef struct
    {
        long int type;
        MSGDATA  val;
    } ZMSG;
    ZMSG m_msgDta;
  public:
    Z_Msg(int key = 0, int type = 1) : m_msgKey(key),m_msgId(-1),m_msgType(type),m_created(0)
    {
        if (key != 0)
        {
            create_object();
            struct msqid_ds info;
            msgctl(m_msgId, IPC_STAT, &info);
            // memset(&m_msgDta, 0x00, sizeof(ZMSG));
            m_msgDta.type = 1;
        }
    }

    virtual ~Z_Msg()
    {
        if(m_created)
        {
            if (-1 != m_msgId)
            {
                zprintf3("Z_Msg destruct!\n");
                delete_object();
            }
            m_created = 0;
        }
    }

    void msg_init(int key = 0, int type = 1)
    {
        m_msgKey  = key;
        m_msgType = type;
    }

    bool get_msg(void)
    {
        m_msgId = msgget(m_msgKey, 0666);

        if (m_msgId == -1)
        {
            zprintf1("get_msg get %d failed!\n", m_msgKey);
            return false;
        }
        zprintf3("get success!\n");
        return true;
    }
    void releaseMsg()
    {
        delete_object();
    }
    bool create_object(void);
    bool delete_object(void);
    bool send_object(MSGDATA data);
    bool receive_object(MSGDATA& val, int mode);
    bool receive_object(void *pdata, int *psize, int mode);
    int  GetMsgKey(void) const;
    bool send_object(MSGDATA data, int type);
    bool send_object(void * pdata, int size, int type = 1);
};

template < class MSGDATA >
bool Z_Msg< MSGDATA >::create_object(void)
{
    zprintf3("Z_Msg create_object1!\n");
    m_msgId = msgget(m_msgKey, 0666 | IPC_CREAT | IPC_EXCL);

    if (-1 == m_msgId)
    {
        if(errno == EEXIST)
            m_msgId = msgget(m_msgKey, 0666 | IPC_CREAT);
        if(-1 == m_msgId)
        {
            zprintf1("Z_Msg m_msgKey %d error!\n", m_msgKey);
            return false;
        }
        else
            return true;
    }
    else
    {
        m_created = 1;
    }
    zprintf3("create_object creat success!\n");
    return true;
}
template < class MSGDATA >
bool Z_Msg< MSGDATA >::delete_object(void)
{
    if (msgctl(m_msgId, IPC_RMID, 0) == -1)
        return false;
    m_msgId = -1;
    return true;
}


template < class MSGDATA >
bool Z_Msg< MSGDATA >::send_object(MSGDATA data)
{
    m_msgDta.type = m_msgType;
    m_msgDta.val  = data;


    if (msgsnd(m_msgId, &m_msgDta, sizeof(MSGDATA), IPC_NOWAIT) == -1)
    {
        struct msqid_ds info;
        zprintf1("Z_Msg send failed!\n");
        msgctl(m_msgId, IPC_STAT, &info);
        zprintf1("发送失败 read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
                info.msg_perm.mode&777, (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
        return false;
    }

    return true;
}
template < class MSGDATA >
bool Z_Msg< MSGDATA >::send_object(MSGDATA data, int type)
{

    m_msgDta.type = type;
    m_msgDta.val  = data;

    if (msgsnd(m_msgId, &m_msgDta, sizeof(MSGDATA), IPC_NOWAIT) == -1)
    {
        struct msqid_ds info;
        zprintf1("send failed222!\n");
        fprintf(stderr, "msgrcv failed width erro222: %d\n", errno);
        msgctl(m_msgId, IPC_STAT, &info);
        zprintf1("发送失败 read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
                 info.msg_perm.mode&777, (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
        return false;
    }
    zprintf3("send success!\n");
    return true;
}
template < class MSGDATA >
bool Z_Msg< MSGDATA >::send_object(void * pdata, int size, int type)
{

    if((size > (int)(sizeof(MSGDATA)))|| (pdata ==NULL))
        return false;
    m_msgDta.type = type;

    memcpy(m_msgDta.val, (char*)pdata, size);

    if (msgsnd(m_msgId, &m_msgDta, size, IPC_NOWAIT) == -1)
    {
        struct msqid_ds info;
        zprintf1("send failed222!\n");
        fprintf(stderr, "msgrcv failed width erro222: %d\n", errno);
        msgctl(m_msgId, IPC_STAT, &info);
        zprintf1("发送失败 read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
                 info.msg_perm.mode&777, (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
        return false;
    }
    zprintf3("send success!\n");
    return true;
}


template < class MSGDATA >
bool Z_Msg< MSGDATA >::receive_object(MSGDATA & val, int mode)
{
    int len;

    len = msgrcv(m_msgId, &m_msgDta, sizeof(MSGDATA), m_msgType, mode);
    if (len == -1)
    {
        struct msqid_ds info;
        zprintf1("Z_Msg receive object error!\n");
        msgctl(m_msgId, IPC_STAT, &info);
        zprintf1("接收失败 read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
                    info.msg_perm.mode&777, (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
        return false;
    }
    val = m_msgDta.val;
    return true;
}
template < class MSGDATA >
bool Z_Msg< MSGDATA >::receive_object(void *pdata,int *psize,int mode)
{

    if(pdata == NULL || psize == NULL)
        return false;
     int len;

    len = msgrcv(m_msgId, &m_msgDta, sizeof(MSGDATA), m_msgType, mode);
    if( len ==-1)
    {
        return false;
    }

    *psize = len;
    memcpy(pdata, &m_msgDta.val, len);
    return true;
}

template < class MSGDATA >
int Z_Msg< MSGDATA >::GetMsgKey(void) const
{
    return m_msgKey;
}

#endif /*__ZMSG_H__*/
