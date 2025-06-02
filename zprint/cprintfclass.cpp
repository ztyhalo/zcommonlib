#include "cprintfclass.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
CPrintfClass::CPrintfClass()
{
}


void CPrintfClass::printf_init(const char * name, int fd)
{
    if(fd == 1)          //标准输出
    {
        m_pfd = stdout;
    }
    else
    {
        if(m_pfd != NULL && m_pfd != stdout)
        {
            fclose(m_pfd);
        }
        m_pfd = NULL;
        if(name != NULL)
        {
          // if(remove(name) == 0 )
          //      printf("Removed %s.", name);
          // else
          //      printf("Removed %s. failed!", name);

            m_pfd = fopen(name, "a+");
            if(m_pfd == NULL)
            {
                printf("file %s open fail\n", name);
            }
        }
    }
}


void CPrintfClass::zprintf(const char * format, ...)
{

    if(m_mark == 0 || m_level < 3) return;
    if(m_pfd != NULL)
    {
        int ret;
        char buf[1024];
        va_list args;

        va_start(args, format);
        ret = vsnprintf(buf, sizeof(buf),format, args);
        va_end(args);
        if(ret > 0)
            m_buf.writeBufData(buf, ret +1);
    }
}

// void PRINTF_CLASS::hprintf(const char * format, ...)
// {
//     va_list args;
//     if(m_mark == 0 || m_level < 4) return;

//     lock();
//     va_start(args, format);
//     vfprintf(stdout, format, args);
//     va_end(args);
//     fflush(stdout);
//     unlock();

// }



void CPrintfClass::timeprintf(const char * format, ...)
{
    if(m_mark == 0 || m_level < 2) return;

    if(m_pfd != NULL)
    {
        va_list args;
        struct tm *p;
        struct timeval tv;
        int ret;
        int ret1;
        char buf[1024];
        va_start(args, format);

        gettimeofday(&tv, NULL);
        p = localtime(&tv.tv_sec);

        ret = snprintf(buf, 1024, "%d-%02d-%02d %02d:%03d:%02d ",
            1900+p->tm_year, 1+p->tm_mon, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);
        if(ret > 0)
        {
            ret1 = vsnprintf(buf+ret, sizeof(buf)-ret,format, args);
            if(ret1 > 0)
                m_buf.writeBufData(buf, ret1 +1 + ret);
        }

        va_end(args);
    }
}

// void PRINTF_CLASS::timemsprintf(const char * format, ...)
// {

//     if(m_mark == 0 || m_level < 1) return;
//     lock();
//     if(m_pfd != NULL)
//     {
//         va_list args;
//         struct tm *p;
//         struct timeval tv;
//         va_start(args, format);

//         gettimeofday(&tv, NULL);
//         p = localtime(&tv.tv_sec);

//         fprintf(m_pfd,"%d-%02d-%02d %02d:%02d:%02d.%06ld ",
//             1900+p->tm_year, 1+p->tm_mon, p->tm_mday,
//             p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
//         vfprintf(m_pfd, format, args);
//         va_end(args);
//         fflush(m_pfd);

//     }
//     unlock();
// }
