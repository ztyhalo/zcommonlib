﻿/*
 * File:   timer_poll.h
 * Author: Administrator
 *
 * 文件监管库V1.2
 */
 
#ifndef __E_POLL_H__
#define __E_POLL_H__
#include <sys/types.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <pthread.h>
// #include <errno.h>
#include "zprint.h"
// #include <vector>
#include "pthclass.h"

#define MAXFDS 8
#define EVENTS 100

class z_poll
{
public:
    int m_epFd;
private:
    int         m_active;
    const int   m_epSize;
    int         m_eventFd;

public:
    explicit z_poll(int max_num= MAXFDS);
    int e_poll_add(int fd);
    int e_poll_add_lt(int fd);
    int e_poll_del(int fd);

    int e_poll_del_lt(int fd);

    int e_poll_deactive();
    void stopPoll();
    int get_epoll_size() const;
    bool setNonBlock (int fd);

    int wait_fd_change(int time);
    virtual ~z_poll();

};



class NCbk_Poll:public z_poll,public Pth_Class
{
public:
     explicit NCbk_Poll(int max):z_poll(max){
         ;
     }
     virtual ~NCbk_Poll(){
         zprintf3("delete NCbk_Poll!\n");
     }
};



 
#endif  /* __E_POLL_H__ */
