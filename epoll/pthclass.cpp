#include "pthclass.h"
// #include "zprint.h"
#include <sys/prctl.h>

Pth_Class::~Pth_Class()
{
    g_debugP->zprintf("destory Pth_Class pid %d name %s!\n", (int)m_pid, m_name.c_str());
    if(m_pid > 0){
        running = 0;
        pthread_cancel(m_pid);
        pthread_join(m_pid, NULL);
        m_pid = 0;
    }
    g_debugP->zprintf("destory Pth_Class delete over!\n");
}

void *Pth_Class::start_thread(void * arg)
{
    // g_debugP->zprintf("zty pid start!\n");
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 4)  // glibc ≥ 2.4 的代码逻辑
    g_debugP->zprintf("glibc version: %d.%d!\n", __GLIBC__, __GLIBC_MINOR__);
#else
    int res = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,   NULL);   //设置立即取消
    if (res != 0)
    {
        perror("Thread pthread_setcancelstate failed");
        exit(EXIT_FAILURE);
    }
#endif
    string name = static_cast<Pth_Class *>(arg)->m_name.substr(0, 15);
    prctl(PR_SET_NAME, name.c_str(), 0, 0, 0);
    static_cast<Pth_Class *>(arg)->run();
    pthread_exit(NULL);
    return NULL;
}

int Pth_Class::start(const string & name)
{
    if(m_pid == 0)
    {
        if(pthread_create(&m_pid, NULL, start_thread,this) == 0)
        {
            running = 1;
            m_name = name;
            g_debugP->zprintf("zty create pid %d name %s!\n", (int)m_pid, name.c_str());
            return 0;
        }
        else
        {
            g_debugP->zprintf("creat pthread failed!\n");
            return -1;
        }
    }
    else
    {
        g_debugP->timemsprintf("pid %d have creat\n",(int)m_pid);
        return -1;
    }
}

int Pth_Class::stop()
{

    g_debugP->zprintf("stop pid %d name %s!\n", (int)m_pid, m_name.c_str());
    if(m_pid > 0)
    {
        running = 0;
        pthread_cancel(m_pid);
        pthread_join(m_pid, NULL);
        m_pid = 0;
    }
    g_debugP->zprintf("stop pid %d end!\n",(int)m_pid);
    return 0;
}

int Pth_Class::waitEnd()
{
    g_debugP->zprintf("waitEnd pid %d name %s!\n", (int)m_pid, m_name.c_str());
    if(m_pid > 0)
    {
        running = 0;
        pthread_join(m_pid, NULL);
        m_pid = 0;
    }
    g_debugP->zprintf("waitEnd pid %d end!\n",(int)m_pid);
    return 0;
}
