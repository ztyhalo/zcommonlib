/*
 * File:   timer_poll.h
 * Author: Administrator
 *
 * 文件监管库V1.2
 */

#include "e_poll.h"
#include <vector>

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


Pth_Class::~Pth_Class()
{
    zprintf3("destory Pth_Class pid %d name %s!\n", (int)m_pid, m_name.c_str());
    if(m_pid > 0){
        running = 0;
        pthread_cancel(m_pid);
        pthread_join(m_pid, NULL);
        m_pid = 0;
    }
    zprintf3("destory Pth_Class delete over!\n");
}

void *Pth_Class::start_thread(void * arg)
{
    zprintf3("zty pid start!\n");
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 4)  // glibc ≥ 2.4 的代码逻辑
    zprintf3("glibc version: %d.%d!\n", __GLIBC__, __GLIBC_MINOR__);
#else
    int res = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,   NULL);   //设置立即取消
    if (res != 0)
    {
        perror("Thread pthread_setcancelstate failed");
        exit(EXIT_FAILURE);
    }
#endif

    static_cast<Pth_Class *>(arg)->run();
    pthread_exit(NULL);
    return NULL;
}

int Pth_Class::start(const string & name)
 {
     if(m_pid == 0)
     {
         if(pthread_create(&m_pid, NULL, start_thread,this) != 0)
         {
             zprintf1("creat pthread failed!\n");
             return -1;
         }
         else
         {
             running = 1;
             m_name = name;
             zprintf3("zty create pid %d name %s!\n", (int)m_pid, name.c_str());
             return 0;
         }
     }
     zprintf1("pid %d have creat\n",(int)m_pid);
     return -1;
 }

 int Pth_Class::stop()
 {

     zprintf3("stop pid %d name %s!\n", (int)m_pid, m_name.c_str());
     if(m_pid > 0)
     {
         running = 0;
         pthread_cancel(m_pid);
         pthread_join(m_pid, NULL);
         m_pid = 0;
     }
     zprintf3("stop pid %d end!\n",(int)m_pid);
     return 0;
}

 int Pth_Class::waitEnd()
 {
     zprintf3("waitEnd pid %d name %s!\n", (int)m_pid, m_name.c_str());
     if(m_pid > 0)
     {
         running = 0;
         pthread_join(m_pid, NULL);
         m_pid = 0;
     }
     zprintf3("waitEnd pid %d end!\n",(int)m_pid);
     return 0;
 }






