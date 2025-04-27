
#include <stdio.h>
#include "mutex_class.h"
#include <errno.h>
#include "zprint.h"


MUTEX_CLASS::MUTEX_CLASS()
{
    pthread_mutex_init(&mut, NULL);
}

MUTEX_CLASS::~MUTEX_CLASS()
{
    int ret = pthread_mutex_destroy(&mut);
    if (ret != 0) {
        // 错误处理
        if (ret == EBUSY) {
            zprintf1("mutex busy!");
        } else if (ret == EINVAL) {
            zprintf1("mutex invalid!\n");
        }
    }
}
bool MUTEX_CLASS::lock(void)
{
    int ret = pthread_mutex_lock(&mut);
    if(ret == 0)
        return true;
    else
    {
        zprintf1("mutex mutex_lock error %d!\n", ret);
        return false;
    }
}
bool MUTEX_CLASS::unlock(void)
{
    int ret = pthread_mutex_unlock(&mut);
    if(ret == 0)
        return true;
    else
    {
        zprintf1("mutex mutex_unlock error %d!\n", ret);
        return false;
    }

}

