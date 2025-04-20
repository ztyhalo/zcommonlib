#ifndef SYSSEM_H
#define SYSSEM_H

#include <sys/sem.h>
#include <sys/ipc.h>

#define Z_MEM_ALIGEN_SIZE(size, align) (((size) +(align) - 1) & ~((align) -1))

union semun
{
    int              val;
    struct semid_ds* buf;
    unsigned short*  array;
};

int create_sem(key_t key, int val);
int new_create_sem(key_t key, int val, int & createdMark);
int init_sem(int sem_id, int init_value);
int del_sem(int sem_id);
int sem_p(int sem_id);
int sem_v(int sem_id);
int sem_p(int sem_id, int milliseconds);

#endif // SYSSEM_H
