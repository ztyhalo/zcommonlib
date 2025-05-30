/****************************************************
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
#include "timers.h"

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
            // create_object();
            get_msg();
            // struct msqid_ds info;
            // msgctl(m_msgId, IPC_STAT, &info);
            // memset(&m_msgDta, 0x00, sizeof(ZMSG));

        }
        m_msgDta.type = type;
    }

    virtual ~Z_Msg()
    {
        if(m_created)
        {
            if (-1 != m_msgId)
            {
                zprintf1("Z_Msg destruct msgid %d m_msgKey %d!\n", m_msgId, m_msgKey);
                delete_object();
            }
            m_created = 0;
        }
        zprintf3("Z_Msg destruct end!\n");
    }

    void msg_init(int key = 0, int type = 1)
    {
        m_msgKey  = key;
        m_msgType = type;
    }

    bool get_msg(void)
    {
        m_msgId = msgget(m_msgKey, 0666);

        if (m_msgId >= 0)
        {
            zprintf3("get %d success id %d!\n", m_msgKey, m_msgId);
            return true;
        }
        else
        {
            zprintf1("get_msg get %d failed!\n", m_msgKey);
            return false;
        }

    }
    void releaseMsg()
    {
        delete_object();
    }
    bool create_object(void);
    bool delete_object(void);
    bool send_object(MSGDATA data);
    bool receive_object(MSGDATA& val, int mode, int & size);
    bool receive_object(void *pdata, int *psize, int mode);
    int  GetMsgKey(void) const;
    bool send_object(MSGDATA data, int type);
    bool send_object(void * pdata, int size, int type = 1);
    bool msgReceiveObject(MSGDATA & val, int & size, int timeover);
};

template < class MSGDATA >
bool Z_Msg< MSGDATA >::create_object(void)
{
    zprintf3("Z_Msg create_object1 key %d!\n", m_msgKey);
    m_msgId = msgget(m_msgKey, 0666 | IPC_CREAT | IPC_EXCL);

    if (-1 != m_msgId)
    {
        m_created = 1;
        zprintf3("create_object creat success!\n");
        return true;
    }
    else
    {
        if(errno == EEXIST)
            m_msgId = msgget(m_msgKey, 0666 | IPC_CREAT);
        if(-1 != m_msgId)
            return true;
        else
        {
            zprintf1("Z_Msg m_msgKey %d error!\n", m_msgKey);
            return false;
        }
    }
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


    if (msgsnd(m_msgId, &m_msgDta, sizeof(MSGDATA), IPC_NOWAIT) != -1)
        return true;
    else
    {
        struct msqid_ds info;
        zprintf1("Z_Msg send failed!\n");
        msgctl(m_msgId, IPC_STAT, &info);
        zprintf1("发送失败 read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
                info.msg_perm.mode&777, (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
        return false;
    }
}

template < class MSGDATA >
bool Z_Msg< MSGDATA >::send_object(MSGDATA data, int type)
{

    m_msgDta.type = type;
    m_msgDta.val  = data;

    if (msgsnd(m_msgId, &m_msgDta, sizeof(MSGDATA), IPC_NOWAIT) != -1)
    {
        zprintf4("send success!\n");
        return true;
    }
    else
    {
        struct msqid_ds info;
        zprintf1("send failed222!\n");
        fprintf(stderr, "msgrcv failed width erro222: %d\n", errno);
        msgctl(m_msgId, IPC_STAT, &info);
        zprintf1("发送失败 read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
                 info.msg_perm.mode&777, (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
        return false;
    }

}
template < class MSGDATA >
bool Z_Msg< MSGDATA >::send_object(void * pdata, int size, int type)
{

    if((size > (int)(sizeof(MSGDATA)))|| (pdata ==NULL))
        return false;
    m_msgDta.type = type;

    memcpy(&m_msgDta.val, (char*)pdata, size);

    if (msgsnd(m_msgId, &m_msgDta, size, IPC_NOWAIT) != -1)
    {
        zprintf4("send success!\n");
        return true;
    }
    else
    {
        struct msqid_ds info;
        zprintf1("send failed222!\n");
        fprintf(stderr, "msgrcv failed width erro222: %d\n", errno);
        msgctl(m_msgId, IPC_STAT, &info);
        zprintf1("发送失败 read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
                 info.msg_perm.mode&777, (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
        return false;
    }

}


template < class MSGDATA >
bool Z_Msg< MSGDATA >::receive_object(MSGDATA & val, int mode, int & size)
{
    int len;

    len = msgrcv(m_msgId, &m_msgDta, sizeof(MSGDATA), m_msgType, mode);
    if (len != -1)
    {
        size = len;
        val = m_msgDta.val;
        return true;
    }
    else
    {
        // struct msqid_ds info;
        zprintf1("Z_Msg receive object error %d!\n", errno);
        size = errno;
        // msgctl(m_msgId, IPC_STAT, &info);
        // zprintf1("接收失败 read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
        //             info.msg_perm.mode&777, (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
        return false;
    }
}

template < class MSGDATA >
bool Z_Msg< MSGDATA >::msgReceiveObject(MSGDATA & val, int & size, int timeout)
{

    if(timeout > 0)
    {
        time_t start = time(NULL);
        while(time(NULL) -  start < timeout)
        {
            int len = msgrcv(m_msgId, &m_msgDta, sizeof(MSGDATA), m_msgType, IPC_NOWAIT);
            if (len != -1)
            {
                size = len;
                val = m_msgDta.val;
                return true;
            }
            else if(errno != ENOMSG)
            {
                zprintf1("Z_Msg receive object error %d!\n", errno);
                size = errno;
                return false;
            }
            USLEEP(100000);
        }
        zprintf1("msgId %d recevie timeover %d!\n", m_msgId, timeout);
        size = -3;
        return false;
    }
    else
    {
        return receive_object(val, 0, size);
    }

}


template < class MSGDATA >
bool Z_Msg< MSGDATA >::receive_object(void *pdata,int *psize,int mode)
{

    if(pdata == NULL || psize == NULL)
        return false;
     int len;

    len = msgrcv(m_msgId, &m_msgDta, sizeof(MSGDATA), m_msgType, mode);
    if( len != -1)
    {
        *psize = len;
        memcpy(pdata, &m_msgDta.val, len);
        return true;
    }
    else
    {
        zprintf1("receive object *pdata error%d!\n", errno);
        return false;
    }


}

template < class MSGDATA >
int Z_Msg< MSGDATA >::GetMsgKey(void) const
{
    return m_msgKey;
}

#endif /*__ZMSG_H__*/
