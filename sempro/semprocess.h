/****************************************************
 *************zmap库**********************
 *Version: 1.0
 *History: 2017.7.7
 *
 ****************************************************/

#ifndef __SEMPROCESS_H__
#define __SEMPROCESS_H__

#include "zmsg.h"
#include "e_poll.h"

using namespace std;

template < class MSGDATA >
class SemSendClass : public Z_Msg< MSGDATA >
{
  public:
    SemSendClass(int key = 0, int type = 1) : Z_Msg< MSGDATA >(key, type)
    {
        ;
    }
    virtual ~SemSendClass()
    {
        zprintf3("SemSendClass destruct!\n");
    }
};

template < class MSGDATA >
class SemRevClass : public Z_Msg< MSGDATA >, public MUTEX_CLASS, public Pth_Class
{

  public:
    SemRevClass(int key = 0, int type = 1)
    {
        this->msg_init(key, type);

        if(!this->create_object())
        {
            zprintf1("SemRevClass create object fail!\n");
        }

    }
    virtual ~SemRevClass()
    {
        zprintf3("SemRevClass destruct!\n");
        if(this->running)
        {
            this->running = 0;
            this->releaseMsg();
            this->waitEnd();
        }
    }
    void sem_rec_start(void)
    {
        start("semrevclass");
    }

    void run(void)
    {
        MSGDATA val;
        while (this->running)
        {
            if (this->receive_object(val, 0) == true) //接收成功
            {
                sem_rec_process(val);
            }
        }
    }
    virtual void sem_rec_process(MSGDATA val)
    {
        Q_UNUSED(val);
        zprintf3("receive sem!\n");
    }
};

#endif /*__SEMPROCESS_H__*/
