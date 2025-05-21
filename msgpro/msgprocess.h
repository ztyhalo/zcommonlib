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
#include "mutex_class.h"

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
class MsgRevClass : public Z_Msg< MSGDATA >, public MUTEX_CLASS, public Pth_Class
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
                msgRecvProcess(val);
            }
            else
            {
                if(len != EINTR)
                    break;
            }
        }
    }
    virtual void msgRecvProcess(MSGDATA val)
    {
        Q_UNUSED(val);
        zprintf3("receive sem!\n");
    }
};

template < class MSGDATA, class F = void >
class MsgRevBackClass : public Z_Msg< MSGDATA >, public MUTEX_CLASS, public Call_B_T<MSGDATA, F>
{
public:
    MsgRevBackClass(int key = 0, int type = 1)
    {
        this->msg_init(key, type);
        if(key > 0)
        {
            if(!this->create_object())
            {
                zprintf1("MsgRevBackClass create object fail!\n");
            }
        }

    }
    virtual ~MsgRevBackClass()
    {
        zprintf3("MsgRevBackClass destruct!\n");
        if(this->running)
        {
            this->running = 0;
            this->releaseMsg();
            this->waitEnd();
        }
    }
    void msgRecvStart(void)
    {
        this->start("Msgrevbackclass");
    }
    // template < class DTYPE, int N, class F >
    void run(void)
    {
        MSGDATA val;
        int     len;
        while (this->running)
        {
            if (this->receive_object(val, 0, len) == true) //接收成功
            {
                if (this->z_callbak != NULL) //执行操作
                {
                    this->z_callbak(this->father, val);
                }

            }
            else
            {
                if(len != EINTR)
                    break;
            }
        }
    }
};

#endif /*__MSGPROCESS_H__*/
