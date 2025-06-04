#ifndef __ZPRINT_H__
#define __ZPRINT_H__

#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "printfconf.h"
#include "printfclass.h"




extern PrintfClass * g_debugP;



#define timeprf(...) do { time_t now;\
                        struct tm timenow;\
                        char stbuf[32];\
    time(&now);\
    localtime_r(&now, &timenow);\
    asctime_r(&timenow,stbuf);\
    stbuf[strlen(stbuf)-1] = '\0';\
    printf("%s ", stbuf);\
    printf( __VA_ARGS__);}while(0)

#define timemsprf(...) do {\
    struct tm *p;\
    struct timeval tv;\
    gettimeofday(&tv, NULL);\
    p = localtime(&tv.tv_sec);\
    printf("%d-%02d-%02d %02d:%02d:%02d.%06ld\n",\
           1900+p->tm_year, 1+p->tm_mon, p->tm_mday,\
           p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);\
    printf( __VA_ARGS__);}while(0)

#define PRINTMODE 2
#if PRINTMODE == 1
    #if PRINT_PRO >=1
    #define zprintf1 g_debugP->timemsprintf
    #else
    #define zprintf1(...)
    #endif

    #if PRINT_PRO >=2
    #define zprintf2 g_debugP->timeprintf
    #else
    #define zprintf2(...)
    #endif

    #if PRINT_PRO >=3
    #define zprintf3 g_debugP->zprintf
    #else
    #define zprintf3(...)
    #endif

    #if PRINT_PRO >=4
    #define zprintf4 g_debugP->hprintf
    #else
    #define zprintf4(...)
    #endif

    #ifndef prop_printf
    #define prop_printf(...)
    #endif
#else
#include "cprintfclass.h"
#ifdef __CPRINTF_CLASS__CPP__
    CPrintfClass *g_pCprintf = CPrintfClass::getInstance();
#else
    extern CPrintfClass *g_pCprintf;
#endif


    #if PRINT_PRO >=1
    #define zprintf1 g_pCprintf->timemsprintf
    #else
    #define zprintf1(...)
    #endif

    #if PRINT_PRO >=2
    #define zprintf2 g_pCprintf->timeprintf
    #else
    #define zprintf2(...)
    #endif

    #if PRINT_PRO >=3
    #define zprintf3 g_pCprintf->zprintf
    #else
    #define zprintf3(...)
    #endif

    #if PRINT_PRO >=4
    #define zprintf4 g_pCprintf->hprintf
    #else
    #define zprintf4(...)
    #endif

    #ifndef prop_printf
    #define prop_printf(...)
    #endif
#endif //PRINTMODE


#endif //__ZPRINT_H__
