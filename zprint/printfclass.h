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
    ~PrintfClass();

  public:

    static PrintfClass * getInstance(void)
    {
        static PrintfClass instance;
        return &instance;
    }
    bool lock();
    bool unlock();
    void printfClassInit(const string & dir, const string & name="");

    void printfInit(const char * name, int fd);
    void zprintf(const char * format, ...);
    void timeprintf(const char * format, ...);
    void timemsprintf(const char * format, ...);
    void hprintf(const char * format, ...);
};

extern PrintfClass * g_debugP;

#endif // PRINTFCLASS_H
