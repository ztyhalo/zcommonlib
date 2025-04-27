#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include "zprint.h"
#include "version.h"




//PRINTF_CLASS *debug_p = new PRINTF_CLASS(DEBUG_F_DIR, 1);
PRINTF_CLASS * PRINTF_CLASS::m_pSelf = NULL;
PRINTF_CLASS *debug_p = PRINTF_CLASS::getInstance();

static int hn_open_m_level_config(const char * file)
{
    char buf[8];
    int i = 0;
    FILE *fp;

    fp = fopen(file, "r");
    if(fp == NULL)
    {
        zprintf1("hn m_level config no!\n");
        return -1;
    }
    memset(buf, 0x00, 8);

    if(fgets(buf, 8, fp) != NULL)
    {
        i = atoi(buf);
    }

    fclose(fp);


    return i;
}

PRINTF_CLASS::PRINTF_CLASS():m_pfd(stdout),m_mark(0),m_level(PRINT_PRO)
{
    pthread_mutex_init(&m_printMut, NULL);
}

PRINTF_CLASS * PRINTF_CLASS::getInstance(void)
{
    if(m_pSelf == NULL)
    {
        m_pSelf = new PRINTF_CLASS();
    }
    return m_pSelf;
}
bool PRINTF_CLASS::lock()
{
    return pthread_mutex_lock(&m_printMut);
}

bool PRINTF_CLASS::unlock()
{
    return pthread_mutex_unlock(&m_printMut);
}

void PRINTF_CLASS::printf_class_init(const string & dir, const string & name)
{

    struct timeval tv;
    char buf[48];


    if(!dir.empty())
    {
        if(dir == "stdout")
        {
            m_mark = 1;
        }
        else
        {
             if (access(dir.c_str(), F_OK) == 0) // dir exist
             {
                string dirlog;
                if(name.empty())
                 {

                    struct tm *p;
                    gettimeofday(&tv, NULL);
                    p = localtime(&tv.tv_sec);
                    memset(buf, 0x00, sizeof(buf));
                    sprintf(buf,"%02d-%02d_%02d_%02d_%02d.log",
                           1+p->tm_mon, p->tm_mday,
                           p->tm_hour, p->tm_min, p->tm_sec);
                    string log = buf;

                    dirlog = dir + log;
                }
                else
                    dirlog = name;
                string lev    = dir + "m_level";

                int val = hn_open_m_level_config(lev.c_str());
                if(val > 0)
                {
                    m_level = val;
                }


                m_pfd =  fopen(dirlog.c_str(), "a+");
                if(m_pfd == NULL)
                {
                    printf("file %s open fail!\n", dirlog.c_str());
                }
                else
                {
                    m_mark = 1;
                    zprintf1("commlib version %d_%d!\n",MAIN_VER, SLAVE_VER);

                }
             }
        }
    }
    else
        m_mark = 0;
}

void PRINTF_CLASS::zprintf(const char * format, ...)
{

    if(m_mark == 0 || m_level < 3) return;
    if(m_pfd != NULL)
    {
        va_list args;
        lock();
        va_start(args, format);
        vfprintf(m_pfd, format, args);
        va_end(args);
        fflush(m_pfd);
        unlock();
    }
}

void PRINTF_CLASS::hprintf(const char * format, ...)
{
    va_list args;
    if(m_mark == 0 || m_level < 4) return;

    lock();
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fflush(stdout);
    unlock();

}

void PRINTF_CLASS::printf_init(const char * name, int fd)
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

void PRINTF_CLASS::timeprintf(const char * format, ...)
{
    if(m_mark == 0 || m_level < 2) return;
     lock();
    if(m_pfd != NULL)
    {
        va_list args;
        struct tm *p;
        struct timeval tv;
        va_start(args, format);

        gettimeofday(&tv, NULL);
        p = localtime(&tv.tv_sec);

        fprintf(m_pfd,"%d-%02d-%02d %02d:%03d:%02d ",
               1900+p->tm_year, 1+p->tm_mon, p->tm_mday,
               p->tm_hour, p->tm_min, p->tm_sec);
        vfprintf(m_pfd, format, args);
        va_end(args);
        fflush(m_pfd);

    }
    unlock();
}

void PRINTF_CLASS::timemsprintf(const char * format, ...)
{

    if(m_mark == 0 || m_level < 1) return;
    lock();
    if(m_pfd != NULL)
    {
        va_list args;
        struct tm *p;
        struct timeval tv;
        va_start(args, format);

        gettimeofday(&tv, NULL);
        p = localtime(&tv.tv_sec);

        fprintf(m_pfd,"%d-%02d-%02d %02d:%02d:%02d.%06ld ",
               1900+p->tm_year, 1+p->tm_mon, p->tm_mday,
               p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
        vfprintf(m_pfd, format, args);
        va_end(args);
        fflush(m_pfd);

    }
    unlock();
}

