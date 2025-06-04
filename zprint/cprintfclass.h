#ifndef CPRINTFCLASS_H
#define CPRINTFCLASS_H
#include "TwoBufPthT.h"
#include <string>
#include "printfconf.h"

using namespace std;


class CPrintfClass : public TwoBufPthT<char, 40, 1024, CPrintfClass>
{
private:
    CPrintfClass();  // 或 protected
    ~CPrintfClass();

public:
    static CPrintfClass* getInstance() {
        static CPrintfClass instance;
        return &instance;
    }

    void printfInit(const string & dir = DEBUG_F_DIR, const string & name = "");
    void zprintf(const char * format, ...);
    void timeprintf(const char * format, ...);
    void timemsprintf(const char * format, ...);
    void hprintf(const char * format, ...);

    FILE * m_pfd;
    int    m_mark;
    int    m_level;
    string m_name;
};

#endif // CPRINTFCLASS_H
