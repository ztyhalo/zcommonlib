#ifndef ZFILE_H
#define ZFILE_H

#include "zprint.h"

class ZFile
{
public:
    ZFile();
    virtual ~ZFile()
    {
        zprintf3("Zfile destruct!\n");
    }
    static bool exists(const string & fileName);
    static bool remove(const string & fileName);
};

#endif // ZFILE_H
