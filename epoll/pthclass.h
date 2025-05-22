#ifndef PTHCLASS_H
#define PTHCLASS_H

#include <sys/types.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <pthread.h>

#include <atomic>
#include <string>
#include "zprint.h"

using namespace std;

class Pth_Class
{
private:
    pthread_t m_pid;
    string    m_name;
public:
    std::atomic<int> running;
private:
    static void * start_thread(void * arg);

public:
    explicit Pth_Class():m_pid(0),m_name(""),running(0){
        ;
    }
    virtual ~Pth_Class();

    int start(const string & name ="");

    int stop();

    int waitEnd();

    virtual void run() = 0;


};

//修改添加单独的callback类，并添加数据长度
template < class DTYPE, class F >
class CallBack_T
{
  public:
    F* m_father;

  public:
    CallBack_T():m_father(NULL),m_callbak(NULL)
    {
        ;
    }
    virtual ~CallBack_T()
    {
        zprintf3("destory CallBack_T!\n");
    }

    int (*m_callbak)(F* pro, DTYPE val,int len);

    int set_z_callback(int (*callback)(F* pro, DTYPE, int), F* arg);

};

template < class DTYPE, class F >
int CallBack_T< DTYPE, F >::set_z_callback(int (*callback)(F* pro, DTYPE, int), F* arg)
{
    if (callback != NULL)
    {
        m_callbak = callback;
        m_father    = arg;
        return 0;
    }
    return -1;
}



//线程回调类
template < class DTYPE, class F >
class Call_B_T : public Pth_Class
{
public:
    F* father;

public:
    Call_B_T();
    virtual ~Call_B_T()
    {
        zprintf3("destory Call_B_T!\n");
        this->running = 0;
    }

    int (*z_callbak)(F* pro, DTYPE val);

    int set_z_callback(int (*callback)(F* pro, DTYPE), F* arg);
    int z_pthread_init(int (*callback)(F* pro, DTYPE), F* arg, const string & pthName="");
};

template < class DTYPE, class F >
Call_B_T< DTYPE, F >::Call_B_T():father(NULL),z_callbak(NULL)
{
    ;
}
template < class DTYPE, class F >
int Call_B_T< DTYPE, F >::set_z_callback(int (*callback)(F* pro, DTYPE), F* arg)
{
    if (callback != NULL)
    {
        z_callbak = callback;
        father    = arg;
        return 0;
    }
    return -1;
}

template < class DTYPE, class F >
int Call_B_T< DTYPE, F >::z_pthread_init(int (*callback)(F* pro, DTYPE), F* arg, const string & pthName)
{
    set_z_callback(callback, arg);
    start(pthName);
    return 0;
}

#endif // PTHCLASS_H
