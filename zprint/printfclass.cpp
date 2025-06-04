#include "printfclass.h"
#include "printfconf.h"
#include "version.h"
#include <stdarg.h>
#include <string.h>
// PrintfClass * PrintfClass::m_pSelf = NULL;
PrintfClass *g_debugP = PrintfClass::getInstance();

static int hnOpenPlevelConfig(const char * file)
{
    char buf[8];
    int i = 0;
    FILE *fp;

    fp = fopen(file, "r");
    if(fp == NULL)
    {
        printf("hn m_level config no!\n");
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

PrintfClass::PrintfClass():m_pfd(stdout),m_mark(0),m_level(PRINT_PRO),m_name(program_invocation_short_name)
{
    printf("PrintfClass init!\n");
    pthread_mutex_init(&m_printMut, NULL);
    printfClassInit(DEBUG_F_DIR,"");
}

PrintfClass::~PrintfClass()
{
    printf("destory PrintfClass!\n");
    if(m_pfd != stdout && m_pfd != NULL)
    {
        fflush(m_pfd);
        // int fd = fileno(m_pfd);
        // fsync(fd);
        fclose(m_pfd);
        m_pfd = NULL;
        int ret = pthread_mutex_destroy(&m_printMut);
        if (ret != 0) {
            // 错误处理
            if (ret == EBUSY) {
                printf("mutex busy!");
            } else if (ret == EINVAL) {
                printf("mutex invalid!\n");
            }
        }
    }
    printf("PrintfClass destruct end!\n");
}



bool PrintfClass::lock()
{
    return pthread_mutex_lock(&m_printMut);
}

bool PrintfClass::unlock()
{
    return pthread_mutex_unlock(&m_printMut);
}

void PrintfClass::printfClassInit(const string & dir, const string & name)
{

    struct timeval tv;
    char buf[256];


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
                string dirlog = dir;
                if(name.empty())
                {

                    string delete_file;
                    delete_file = "find " + dir + " -type f -mtime +30 -exec rm -rf {} \\;";
                    system(delete_file.c_str( ));

                    struct tm *p;
                    gettimeofday(&tv, NULL);
                    p = localtime(&tv.tv_sec);
                    memset(buf, 0x00, sizeof(buf));
                    sprintf(buf,"%s_%04d_%02d-%02d_%02d_%02d_%02d.log",
                        m_name.c_str( ), 1900 + p->tm_year, 1+p->tm_mon, p->tm_mday,
                        p->tm_hour, p->tm_min, p->tm_sec);
                    string log = buf;

                    dirlog += log;
                }
                else
                    dirlog += name;
                string lev    = dir + "m_level";

                int val = hnOpenPlevelConfig(lev.c_str());
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
                    timemsprintf("commlib version %d_%d!\n",MAIN_VER, SLAVE_VER);

                }
            }
        }
    }
    else
        m_mark = 0;
}

void PrintfClass::zprintf(const char * format, ...)
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

void PrintfClass::hprintf(const char * format, ...)
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

void PrintfClass::printfInit(const char * name, int fd)
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

void PrintfClass::timeprintf(const char * format, ...)
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

        fprintf(m_pfd,"%d-%02d-%02d %02d:%02d:%02d ",
            1900+p->tm_year, 1+p->tm_mon, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);
        vfprintf(m_pfd, format, args);
        va_end(args);
        fflush(m_pfd);

    }
    unlock();
}

void PrintfClass::timemsprintf(const char * format, ...)
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

int  PrintfClass::writeData(char * buf, int len)
{
    lock();
    if(m_pfd != NULL)
    {
        int ret = fwrite(buf, 1, len, m_pfd);
        // int ret =   fprintf(m_pfd,"%s", buf);
        // fflush(m_pfd);
        unlock();
        return ret;
    }
    unlock();
    return -1;
}
