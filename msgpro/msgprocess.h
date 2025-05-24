/****************************************************
 *************zmap库**********************
 *Version: 1.0
 *History: 2017.7.7
 *
 ****************************************************/

#ifndef __MSGPROCESS_H__
#define __MSGPROCESS_H__

#include "zmsg.h"
// #include "e_poll.h"
#include "pthclass.h"
// #include "mutex_class.h"

using namespace std;

template < class MSGDATA >
class MsgSendClass : public Z_Msg< MSGDATA >
{
  public:
    MsgSendClass(int key = 0, int type = 1) : Z_Msg< MSGDATA >(key, type)
    {
        ;
    }
    virtual ~MsgSendClass()
    {
        zprintf3("MsgSendClass destruct!\n");
    }
};

template < class MSGDATA >
class MsgRevClass : public Z_Msg< MSGDATA >,public Pth_Class
{

  public:
    MsgRevClass(int key = 0, int type = 1)
    {
        this->msg_init(key, type);
        if(key > 0)
        {
            if(!this->create_object())
            {
                zprintf1("SemRevClass create object fail!\n");
            }
        }

    }
    virtual ~MsgRevClass()
    {
        zprintf3("SemRevClass destruct!\n");
        if(this->running)
        {
            this->running = 0;
            this->releaseMsg();
            this->waitEnd();
        }
    }
    void msgRecvStart(void)
    {
        start("Msgrevclass");
    }

    void run(void)
    {
        MSGDATA val;
        int     len;
        while (this->running)
        {
            if (this->receive_object(val, 0, len) == true) //接收成功
            {
                msgRecvProcess(val, len);
            }
            else
            {
                if(len != EINTR)
                    break;
            }
        }
    }
    virtual void msgRecvProcess(MSGDATA val, int len)
    {
        Q_UNUSED(val);
        (void)len;
        zprintf3("receive sem!\n");
    }
};

template < class MSGDATA, class F = void >
class MsgRevBackClass : public CallBack_T<MSGDATA, F>, public MsgRevClass<MSGDATA>
{
public:
    MsgRevBackClass(int key = 0, int type = 1):MsgRevClass<MSGDATA>(key, type)
    {
        ;
    }
    virtual ~MsgRevBackClass()
    {
        zprintf3("MsgRevBackClass destruct!\n");
    }

    void msgRecvProcess(MSGDATA val, int len) override
    {
        if (this->m_callbak != NULL) //执行操作
        {
            this->m_callbak(this->m_father, val, len);
        }
    }
    int msgPthreadInit(int (*callback)(F* pro, MSGDATA, int), F* arg, const string & pthName="")
    {
        this->set_z_callback(callback, arg);
        this->start(pthName);
        return 0;
    }
};

#endif /*__MSGPROCESS_H__*/
