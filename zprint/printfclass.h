#ifndef PRINTFCLASS_H
#define PRINTFCLASS_H
#include <string>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <time.h>
#include <sys/time.h>

using namespace std;

class PrintfClass
{
  private:
    FILE * m_pfd;
    int    m_mark;
    int    m_level;
    pthread_mutex_t m_printMut;
    PrintfClass();
    class AutoRelease {
      public:
        ~AutoRelease()
        {
            if(PrintfClass::m_pSelf != NULL)
            {
                delete PrintfClass::m_pSelf;
                PrintfClass::m_pSelf = NULL;
            }
        }
    };
    static PrintfClass * m_pSelf;
    static AutoRelease m_pAutoRelease;

  public:

    ~PrintfClass()
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
    bool lock();
    bool unlock();
    void printfClassInit(const string & dir, const string & name="");
    static PrintfClass * getInstance(void);
    void printfInit(const char * name, int fd);
    void zprintf(const char * format, ...);
    void timeprintf(const char * format, ...);
    void timemsprintf(const char * format, ...);
    void hprintf(const char * format, ...);
};

extern PrintfClass * g_debugP;

#endif // PRINTFCLASS_H
