#ifndef COM_DATE_H
#define COM_DATE_H

#include <unistd.h>
#include <time.h>
#include <string>
#include <sys/time.h>
#include "zprint/zprint.h"

using namespace std;
class Com_Date
{
public:
    time_t now;
    struct tm tm;

public:
    Com_Date();
    virtual ~Com_Date()
    {
        zprintf3("com_data destruct!\n");
    }
    void gen_date_string (string & date_string);

};

#endif // COM_DATE_H
