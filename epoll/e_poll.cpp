/*
 * File:   timer_poll.h
 * Author: Administrator
 *
 * 文件监管库V1.2
 */

#include "e_poll.h"
#include <vector>
#include <sys/prctl.h>

#define MAXFDS 8
#define EVENTS 100


z_poll::z_poll(int max_num):m_epSize(max_num)
{
    m_epFd = epoll_create(max_num);
    if(m_epFd == -1)
    {
        zprintf1("epoll creat failed!\n");
        m_active = 0;
        return;
    }
    m_eventFd = eventfd(0, EFD_NONBLOCK);
    if(m_eventFd == -1)
    {
        zprintf1("z_poll faile create eventfd!\n");
    }
    else if(e_poll_add_lt(m_eventFd) != 0)
    {
        close(m_eventFd);
        m_eventFd = -1;
    }
    m_active = 1;
}

int z_poll::e_poll_add(int fd)
{
    if(setNonBlock(fd) == false)
        return -1;
    int err = 0;
    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;
    err = epoll_ctl(m_epFd, EPOLL_CTL_ADD, fd, &ev);
    if(err == -1)
    {
        zprintf1("%s\n",strerror(errno));
        return err;
    }
    return 0;
}

int z_poll::e_poll_add_lt(int fd)
{
    if(setNonBlock(fd) == false)
        return -1;
    int                err = 0;
    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.data.fd = fd;
    ev.events  = EPOLLIN;
    err        = epoll_ctl(m_epFd, EPOLL_CTL_ADD, fd, &ev);
    if(err == -1)
    {
        zprintf1("%s\n", strerror(errno));
        return err;
    }
    return 0;
}

int z_poll::e_poll_del(int fd)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;
    int err = epoll_ctl(m_epFd, EPOLL_CTL_DEL, fd, &ev);
    if(err == -1)
    {
        zprintf1("%s\n",strerror(errno));
        return err;
    }

    return 0;
}

int z_poll::e_poll_del_lt(int fd)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events  = EPOLLIN;
    int err    = epoll_ctl(m_epFd, EPOLL_CTL_DEL, fd, &ev);
    if(err == -1)
    {
        zprintf1("epoll_ctl error %s\n", strerror(errno));
        return err;
    }

    return 0;
}

int z_poll::e_poll_deactive()
{
    m_active = 0;
    return 0;
}
void z_poll::stopPoll()
{
    if(m_eventFd > 0)
    {
        uint64_t value = 1;
        write(m_eventFd, &value, sizeof(value));
    }
    else
    {
        printf("m_eventFd error!\n");
        close(m_epFd);
    }
}
int z_poll::get_epoll_size() const
{
    return m_epSize;
}
bool z_poll::setNonBlock (int fd)
{
     int flags = fcntl (fd, F_GETFL, 0);
     flags |= O_NONBLOCK;
     if (-1 == fcntl (fd, F_SETFL, flags))
     {
         zprintf1("fd%d set non block failed!\n", fd);
         return false;
     }

     return true;
}

int z_poll::wait_fd_change(int time)
{
    // struct epoll_event events[this->get_epoll_size()];
    // memset(&events, 0, sizeof(events));
    std::vector<struct epoll_event> events(m_epSize);

     int nfds = epoll_wait(m_epFd, events.data(), get_epoll_size(), time);

     if(nfds > 0)
     {
        for(int i = 0; i < nfds; ++i)
         {
             if(events[i].data.fd == m_eventFd)
                  return -1;
         }
         return nfds;
     }
     else
         return -2;
}

z_poll::~z_poll()
{
    zprintf3("destory zpoll!\n");

    if(m_active)
    {

        if(m_epFd)
            close(m_epFd);
        if(m_eventFd > 0)
            close(m_eventFd);
        m_eventFd = -1;
        m_active = 0;
        m_epFd = 0;

    }
}









