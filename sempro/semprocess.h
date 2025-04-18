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
};

template < class MSGDATA >
class SemRevClass : public Z_Msg< MSGDATA >, public MUTEX_CLASS
{
  private:
    Z_PTH< SemRevClass > pth;

  public:
    SemRevClass(int key = 0, int type = 1)
    {
        qDebug() << "SemRevClass1";
        this->msg_init(key, type);
        //        if(this->get_msg())
        //        {
        //            this->delete_object();
        //        }
        //                zprintf3("receive sem creat start!\n");
        qDebug() << "SemRevClass2";
        this->create_object();

        pth.pthread_init(this, &SemRevClass::pth_exe);
        qDebug() << "create_object3";
    }

    void sem_rec_start(void)
    {
        pth.start();
    }

    void pth_exe(void)
    {
        MSGDATA val;
        while (1)
        {
            if (this->receive_object(val, 0) == true) //接收成功
            {
                sem_rec_process(val);
            }
            //            usleep(100);
        }
    }
    virtual void sem_rec_process(MSGDATA val)
    {
        Q_UNUSED(val);
        zprintf3("receive sem!\n");
    }
};

#endif /*__SEMPROCESS_H__*/
