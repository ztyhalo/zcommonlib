#ifndef CPRINTFCLASS_H
#define CPRINTFCLASS_H
#include "TwoBufPthT.h"

class CPrintfClass
{
  public:
    TwoBufPthT<char, 20, 1024, CPrintfClass>  m_buf;
    FILE * m_pfd;
    int    m_mark;
    int    m_level;
  public:
    CPrintfClass();
    void printf_init(const char * name, int fd);
    void zprintf(const char * format, ...);
    void timeprintf(const char * format, ...);
    void timemsprintf(const char * format, ...);
    void hprintf(const char * format, ...);
};

#endif // CPRINTFCLASS_H
