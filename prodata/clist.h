/****************************************************
 *************clist库**********************
 *Version: 1.2
 *History: 2017.7.7
 *         2025.4.18
 ****************************************************/

#ifndef __CLIST_H__
#define __CLIST_H__
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/msg.h>
#include <pthread.h>
#include <semaphore.h>
#include "zprint/zprint.h"

using namespace std;

template < class DTYPE >
class List_N
{
  public:
    DTYPE*  p;
    List_N* next;
};

template < class DTYPE, int N = 2 >
class C_LIST_T
{
  public:
    DTYPE            buf[N];
    List_N< DTYPE >  p[N];
    List_N< DTYPE >* rw_P;
    List_N< DTYPE >* free_P;
    pthread_mutex_t list_mut;

  public:
    C_LIST_T()
    {
        memset(buf, 0x00, sizeof(buf));
        for (int i = 0; i < N - 1; i++)
        {
            p[i].next = &p[i + 1];
            p[i].p    = &buf[i];
        }
        p[N - 1].next = NULL;
        p[N - 1].p    = &buf[N - 1];
        free_P        = p;
        rw_P          = NULL;
        pthread_mutex_init(&list_mut, NULL);
    }
    virtual ~C_LIST_T()
    {
        zprintf3("C_LIST_T destruct!\n");
        pthread_mutex_unlock(&list_mut);
    }

    int    buf_write_data(const DTYPE & val);
    int    buf_write_data(const DTYPE* val);
    int    compare_fun(const DTYPE& f, const DTYPE& c);
    int    delete_list_data(const DTYPE& val);
    int    delete_list_data(const DTYPE* val);
    int    condition_delete_list_data(int (*condition_fun)(const DTYPE&, const DTYPE&), const DTYPE& val);
    DTYPE* buf_read_data(int (*condition_fun)(const DTYPE&, const DTYPE&), const DTYPE& val);

    List_N< DTYPE >* get_list_head(void)
    {
        return rw_P;
    }
};

template < class DTYPE, int N >
int C_LIST_T< DTYPE, N >::buf_write_data(const DTYPE & val)
{
    List_N< DTYPE >* midpoint = NULL;
    int              err      = 0;

    pthread_mutex_lock(&list_mut);
    if (free_P != NULL)
    {
        midpoint       = free_P;
        free_P         = free_P->next;
        *(midpoint->p) = val;
        midpoint->next = rw_P;
        rw_P           = midpoint;
    }
    else
    {
        err = -1;
    }
    pthread_mutex_unlock(&list_mut);

    return err;
}

template < class DTYPE, int N >
int C_LIST_T< DTYPE, N >::buf_write_data(const DTYPE* val)
{
    List_N< DTYPE >* midpoint = NULL;
    int              err      = 0;

    pthread_mutex_lock(&list_mut);
    if (free_P != NULL)
    {
        midpoint       = free_P;
        free_P         = free_P->next;
        *(midpoint->p) = *val;
        midpoint->next = rw_P;
        rw_P           = midpoint;
    }
    else
    {
        err = -1;
    }
    pthread_mutex_unlock(&list_mut);
    return err;
}
template < class DTYPE, int N >
int C_LIST_T< DTYPE, N >::compare_fun(const DTYPE & f, const DTYPE & c)
{
    return f == c ? 0 : -1;
}

template < class DTYPE, int N >
int C_LIST_T< DTYPE, N >::condition_delete_list_data(int (*condition_fun)(const DTYPE&, const DTYPE&), const DTYPE& val)
{
    if (condition_fun == NULL)
        return -1;
    int              err      = 0;
    List_N< DTYPE >* midpoint = NULL;
    List_N< DTYPE >* copymid  = NULL;
    List_N< DTYPE >* retval   = NULL;

    pthread_mutex_lock(&list_mut);

    midpoint = rw_P;
    if (midpoint == NULL)
        err = -3;
    while (midpoint != NULL)
    {

        if (condition_fun(*(midpoint->p), val) == 0)
        {
            copymid = midpoint;
            break;
        }
        retval   = midpoint;
        midpoint = midpoint->next;
    }
    if (copymid != NULL)
    {
        if (retval == NULL)
        {
            rw_P          = copymid->next;
            copymid->next = free_P;
            free_P        = copymid;
        }
        else
        {
            retval->next  = copymid->next;
            copymid->next = free_P;
            free_P        = copymid;
        }
    }
    else
        err = -2;
    pthread_mutex_unlock(&list_mut);
    return err;
}

template < class DTYPE, int N >
int C_LIST_T< DTYPE, N >::delete_list_data(const DTYPE& val)
{
    return condition_delete_list_data(this->compare_fun, val);
}

template < class DTYPE, int N >
DTYPE* C_LIST_T< DTYPE, N >::buf_read_data(int (*condition_fun)(const DTYPE&, const DTYPE&), const DTYPE& val)
{
    if (condition_fun == NULL)
        return NULL;

    List_N< DTYPE >* midpoint = NULL;

    DTYPE* retval = NULL;

    pthread_mutex_lock(&list_mut);
    midpoint = rw_P;
    while (midpoint != NULL)
    {
        if (condition_fun(*(midpoint->p), val) == 0)
        {
            retval = midpoint->p;
        }
        midpoint = midpoint->next;
    }
    pthread_mutex_unlock(&list_mut);
    return retval;
}

template < class DTYPE, int N >
int C_LIST_T< DTYPE, N >::delete_list_data(const DTYPE* val)
{
    List_N< DTYPE >* midpoint = NULL;
    List_N< DTYPE >* copymid  = NULL;
    int              err      = 0;

    pthread_mutex_lock(&list_mut);
    if (val != NULL && rw_P != NULL)
    {
        for (midpoint = rw_P; midpoint != NULL && midpoint->p != val; midpoint = midpoint->next)
        {
            copymid = midpoint;
        }
        if (midpoint->p == val)
        {
            if (copymid == NULL)
            {
                rw_P = midpoint->next;
            }
            else
            {
                copymid->next = midpoint->next;
            }
            midpoint->next = free_P;
            free_P         = midpoint;
        }
        else
        {
            zprintf1("delete val  failed\n");
            err = -2;
        }
    }
    else
    {
        err = -1;
        zprintf1("zrw_p is full\n");
    }
    pthread_mutex_unlock(&list_mut);

    return err;
}

#endif /*__CLIST_H__*/
