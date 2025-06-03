#include "cprintfclass.h"
// #include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include "version.h"
#include "printfconf.h"


// CPrintfClass * CPrintfClass::m_pCSelf = NULL;
CPrintfClass *g_pCprintf = CPrintfClass::getInstance();



int writePrintfCallBack(CPrintfClass* pro, char * val,int len)
{
    if(pro != NULL && val != NULL && len > 0 && pro->m_pfd != NULL)
    {
        int writelen = fwrite(val, 1, len, pro->m_pfd);
        if(writelen != len)
        {
            printf("printf write error %d:%d!\n", len, writelen);
            return -1;
        }
        fflush(pro->m_pfd);
        return 0;
    }
    else
        return -2;
}

CPrintfClass::CPrintfClass():m_pfd(stdout),m_mark(0),m_level(PRINT_PRO),m_name(program_invocation_short_name)
{
    ;
}

// CPrintfClass * CPrintfClass::getInstance(void)
// {
//     if(m_pCSelf == NULL)
//     {
//         m_pCSelf = new CPrintfClass();
//     }
//     return m_pCSelf;
// }

CPrintfClass::~CPrintfClass()
{
    printf("destory CPrintfClass!\n");
    lock_guard<MUTEX_CLASS> locker(m_mutex);
    if(m_pfd != stdout && m_pfd != NULL)
    {
        fflush(m_pfd);
        fclose(m_pfd);
        m_pfd = NULL;
    }
    printf("CPrintfClass destruct end!\n");
}

static int printLevelConfig(const char * file)
{
    char buf[8];
    int i = 0;
    FILE *fp;

    fp = fopen(file, "r");
    if(fp == NULL)
    {
        g_pCprintf->zprintf("hn m_level config no!\n");
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


void CPrintfClass::printfInit(const string & dir, const string & name)
{

    struct timeval tv;
    char buf[256];

    // lock_guard<MUTEX_CLASS> locker(m_mutex);
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

                int val = printLevelConfig(lev.c_str());
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
                    this->setCallback(writePrintfCallBack, this);
                    this->start("cprintfpth");
                    g_pCprintf->timemsprintf("commlib version %d_%d!\n",MAIN_VER, SLAVE_VER);

                }

            }
        }
    }
    else
        m_mark = 0;
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
            this->writeBufData(buf, ret);
    }
}





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

        ret = snprintf(buf, 1024, "%d-%02d-%02d %02d:%02d:%02d ",
            1900+p->tm_year, 1+p->tm_mon, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);
        if(ret > 0)
        {
            ret1 = vsnprintf(buf + ret, sizeof(buf)-ret,format, args);
            if(ret1 > 0)
                this->writeBufData(buf, ret1 + ret);
        }

        va_end(args);
    }
}

void CPrintfClass::timemsprintf(const char * format, ...)
{

    if(m_mark == 0 || m_level < 1) return;

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

        ret = snprintf(buf, 1024, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
            1900+p->tm_year, 1+p->tm_mon, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
        if(ret > 0)
        {
            ret1 = vsnprintf(buf+ret, sizeof(buf)-ret,format, args);
            if(ret1 > 0)
                this->writeBufData(buf, ret1 + ret);
        }

        va_end(args);

    }
}

void CPrintfClass::hprintf(const char * format, ...)
{
    va_list args;
    if(m_mark == 0 || m_level < 4) return;

    lock_guard<MUTEX_CLASS> locker(m_mutex);
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fflush(stdout);

}
