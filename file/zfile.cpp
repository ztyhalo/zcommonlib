#include "zfile.h"

ZFile::ZFile()
{
    ;
}

bool ZFile::exists(const string & fileName)
{
    if (access(fileName.c_str(), F_OK) != -1)
    {
       return true;
    }
    else
    {
        return false;
    }
}

bool ZFile::remove(const string & fileName)
{
    if(::remove(fileName.c_str()))
        return true;
    else
        return false;
}
