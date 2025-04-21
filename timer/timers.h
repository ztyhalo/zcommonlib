/******************************************************************
 * File:   timer_poll.h
 * Author: Administrator
 *
 * linux定时器库V1.1
 * history:
 *          2017.7.6 修改timerevent中的成员，去除timer_internal和repeat
 ********************************************************************/

#ifndef TIMERS_H
#define TIMERS_H

#include <map>
#include "e_poll.h"


#define  TIMER_SIZE_MAX           16

template <class FROM = void, class TO = void>
class TimerEvent
{
public:
    FROM *  father;
    TO   *  slot;
    void *  para;

    int     m_filed;
    int  (*t_back)(TimerEvent *);
public:
    TimerEvent(int  (*callback)(TimerEvent *) = NULL,
           FROM * f = NULL, void * arg = NULL, TO * t = NULL )
    {
        m_filed = 0;
        m_filed = timerfd_create(CLOCK_REALTIME, 0);
        if(m_filed <= 0)
        {
            zprintf1("TimerEvent create timerfd fail!\n");
            return;
        }
        t_back = callback;
        father = f;
        slot = t;
        para = arg;
    }

    virtual ~TimerEvent(){    //此处不能关闭filed，由于Ftimer添加时会析构，从而关闭该文件
        if(m_filed >= 0)
        {
            close(m_filed);
            m_filed = -1;
        }

    }

    int timer_start(double timer_internal, bool repeat = true){
        struct itimerspec ptime_internal;
        memset(&ptime_internal, 0x00, sizeof(ptime_internal));
        ptime_internal.it_value.tv_sec = (int) timer_internal;
        ptime_internal.it_value.tv_nsec = (timer_internal - (int) timer_internal)*1000000000;
        if(repeat)
        {
            ptime_internal.it_interval.tv_sec = ptime_internal.it_value.tv_sec;
            ptime_internal.it_interval.tv_nsec = ptime_internal.it_value.tv_nsec;
        }

        timerfd_settime(m_filed, 0, &ptime_internal, NULL);
        return 0;
    }
};


template <class FROM = void, class TO = void>
class F_Timer:public NCbk_Poll, public MUTEX_CLASS
{
public:
    std::map<int, TimerEvent<FROM, TO> *> poll_map;
public:
    F_Timer(int max = 20):NCbk_Poll(max){
        ;
    }

    virtual ~F_Timer(){
        this->running = 0;
        lock();
        typename std::map<int, TimerEvent<FROM, TO> *>::iterator it;
        it = poll_map.begin();
        while(it != poll_map.end())
        {
            // delete_event(it->first);
            if(it->first > 0)
                e_poll_del_lt(it->first);
            if(it->second != NULL)
                delete it->second;
            ++it;
        }
        unlock();
    }
    int add_event(double internal_value,int  (*callback)(TimerEvent<FROM, TO> *) = NULL,
                  FROM * f = NULL,void * arg = NULL,bool rep = true, TO * t = NULL )
    {
        zprintf3("timer is %f!\n", internal_value);
        TimerEvent<FROM, TO> *pMid = new TimerEvent<FROM, TO>(callback,f ,arg, t);
        if(pMid->m_filed > 0){
            lock();
            e_poll_add_lt(pMid->m_filed);
            poll_map.insert(std::pair<int, TimerEvent<FROM, TO> *>(pMid->m_filed, pMid));
            pMid->timer_start(internal_value, rep);
            unlock();
        }
        return pMid->m_filed;
    }
    int delete_event(int event)
    {
        int err = 0;
        if(event <= 0) return -1;
        lock();
        err = e_poll_del_lt(event);
        close(event);
        poll_map.erase(event);
        unlock();
        return err;
    }

    void run(){
        struct epoll_event events[get_epoll_size()];

        uint64_t exp;

        while (this->running)
        {
            memset(&events, 0, sizeof(events));
            int nfds = epoll_wait(m_epFd, events, get_epoll_size(), 5000);
            if (nfds == 0 || nfds == -1)
            {
                zprintf1("epoll wait overtime %d\r\n", nfds);
            }
            lock();
            for (int i = 0; i < nfds; ++i)
            {
                typename std::map<int, TimerEvent<FROM, TO> *>::iterator itmp = poll_map.find(events[i].data.fd);
                if (itmp != poll_map.end())
                {
                    if(read(events[i].data.fd, &exp, sizeof(uint64_t)) ==  sizeof(uint64_t))
                    {
                        itmp->second->t_back(itmp->second);
                    }
                }
            }
            unlock();
        }
    }
};

typedef F_Timer<void,void>    O_Timer;
typedef TimerEvent<void,void> TEvent;
#define B_Timer(x)       F_Timer<x,void>
#define B_TEvent(x)      TimerEvent<x,void>

void linuxDly(int s, int ms);
void linuxDly(int ms);
int set_delay_ts(struct timespec * ts, int sec);
#endif // TIMERS_H

