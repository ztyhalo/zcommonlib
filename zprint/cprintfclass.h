#ifndef CPRINTFCLASS_H
#define CPRINTFCLASS_H
#include "TwoBufPthT.h"
#include <string>

using namespace std;

// class CPrintfClass:public TwoBufPthT<char, 20, 1024, CPrintfClass>
// {
//   private:
//     CPrintfClass();

//     class AutoRelease {
//       public:
//         ~AutoRelease()
//         {
//             printf("delete AutoRelease!\n");
//             if(CPrintfClass::m_pCSelf != NULL)
//             {
//                 delete CPrintfClass::m_pCSelf;
//                 CPrintfClass::m_pCSelf = NULL;
//             }
//         }
//     };
//     static CPrintfClass * m_pCSelf;
//     static AutoRelease m_cAutoRelease;
//   public:
//     // TwoBufPthT<char, 20, 1024, CPrintfClass>  m_buf;
//     FILE * m_pfd;
//     int    m_mark;
//     int    m_level;
//     string m_name;
//   public:
//     ~CPrintfClass();
//     static CPrintfClass * getInstance(void);
//     void printfInit(const string & dir, const string & name);
//     void zprintf(const char * format, ...);
//     void timeprintf(const char * format, ...);
//     void timemsprintf(const char * format, ...);
//     void hprintf(const char * format, ...);
// };

class CPrintfClass : public TwoBufPthT<char, 20, 1024, CPrintfClass>
{
private:
    CPrintfClass();  // 或 protected
    ~CPrintfClass();

public:
    static CPrintfClass* getInstance() {
        static CPrintfClass instance;
        return &instance;
    }

    void printfInit(const string & dir, const string & name);
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
