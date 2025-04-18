/****************************************************
 *************zmap库**********************
 *Version: 1.0
 *History: 2017.7.7
 *
 ****************************************************/

#ifndef __ZMSG_H__
#define __ZMSG_H__
#include <sys/types.h>
#include <map>
#include <string.h>
#include <sys/msg.h>
#include <../zprint/zprint.h>
#include <QDebug>
using namespace std;

template < class MSGDATA >
class Z_Msg
{
  private:
    int msg_key;
    int msg_id;
    int msg_type;

  public:
    typedef struct
    {
        long int type;
        MSGDATA  val;
    } ZMSG;

  public:
    Z_Msg(int key = 0, int type = 1) : msg_key(key), msg_type(type)
    {
        if (key != 0)
        {
            create_object();
            struct msqid_ds info;
            msgctl(msg_id, IPC_STAT, &info);
            //           zprintf3("read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
            //                   info.msg_perm.mode&777,
            //                   (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
        }
    }

    ~Z_Msg()
    {
        if (msg_key != 0)
        {
            //            qDebug()<<"~Z_Msg()";
            delete_object();
        }
    }

    void msg_init(int key = 0, int type = 1)
    {
        msg_key  = key;
        msg_type = type;
    }

    bool get_msg(void)
    {
        msg_id = msgget(msg_key, 0666);

        if (msg_id == -1)
        {
            zprintf3("get failed!\n");
            return false;
        }
        zprintf3("get success!\n");
        return true;
    }

    bool create_object(void);
    bool delete_object(void);
    bool send_object(MSGDATA data);
    bool receive_object(MSGDATA& val, int mode);
    int  GetMsgKey(void);
    bool send_object(MSGDATA data, int type);
};

template < class MSGDATA >
bool Z_Msg< MSGDATA >::create_object(void)
{
    qDebug() << "create_object1";
    msg_id = msgget(msg_key, 0666 | IPC_CREAT);

    if (msg_id == -1)
    {
        //        zprintf3("creat failed!\n");
        return false;
    }
    //    zprintf3("creat success!\n");
    qDebug() << "create_object2";
    return true;
}
template < class MSGDATA >
bool Z_Msg< MSGDATA >::delete_object(void)
{
    //     qDebug()<<"bool Z_Msg<MSGDATA>::~delete_object(void)";
    if (msgctl(msg_id, IPC_RMID, 0) == -1)
        return false;
    return true;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>

template < class MSGDATA >
bool Z_Msg< MSGDATA >::send_object(MSGDATA data)
{
    ZMSG v;
    v.type = msg_type;
    v.val  = data;

    struct msqid_ds info;
    if (msgsnd(msg_id, &v, sizeof(MSGDATA), IPC_NOWAIT) == -1)
    {
        //        qDebug()<<"send failed222";
        zprintf3("send failed111!\n");
        // fprintf(stderr, "msgrcv failed width erro1111: %d\n", errno);
        msgctl(msg_id, IPC_STAT, &info);
        // zprintf3("发送失败 read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
        //       info.msg_perm.mode&777, (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
        return false;
    }
    msgctl(msg_id, IPC_STAT, &info);
    // zprintf3("发送成功：read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
    //         info.msg_perm.mode&777, (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
    // printf("send success!\n");
    return true;
}
template < class MSGDATA >
bool Z_Msg< MSGDATA >::send_object(MSGDATA data, int type)
{
    ZMSG v;
    v.type = type;
    v.val  = data;

    if (msgsnd(msg_id, &v, sizeof(MSGDATA), IPC_NOWAIT) == -1)
    {
        zprintf3("send failed222!\n");
        fprintf(stderr, "msgrcv failed width erro222: %d\n", errno);
        return false;
    }
    printf("send success!\n");
    return true;
}

template < class MSGDATA >
bool Z_Msg< MSGDATA >::receive_object(MSGDATA& val, int mode)
{
    int len;

    ZMSG v;

    struct msqid_ds info;
    len = msgrcv(msg_id, &v, sizeof(MSGDATA), msg_type, mode);
    if (len == -1)
    {
        // zprintf3("receive failed!\n");
        msgctl(msg_id, IPC_STAT, &info);
        // zprintf3("接收失败 read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
        //       info.msg_perm.mode&777, (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
        return false;
    }
    msgctl(msg_id, IPC_STAT, &info);
    // zprintf3("接收成功 read-write:%03o,cbytes=%lu,qnum=%lu,qbytes =%lu\n",
    //      info.msg_perm.mode&777, (ulong)info.__msg_cbytes,(ulong)info.msg_qnum,(ulong)info.msg_qbytes);
    val = v.val;
    return true;
}

template < class MSGDATA >
int Z_Msg< MSGDATA >::GetMsgKey(void)
{
    return msg_key;
}

#endif /*__ZMSG_H__*/
