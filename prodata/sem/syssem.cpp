#include <stdio.h>
// #include <stdlib.h>
#include <errno.h>
#include "syssem.h"
#include "zprint.h"

int create_sem(key_t key, int val)
{
    int semid;

    semid = semget(key, 1, 0666);

    if (semid == -1)
    {
        if (errno == ENOENT)
        {
            semid = semget(key, 1, IPC_CREAT | 0666);
            if (semid == -1)
            {
                perror("Sem Create Error!\n");
                return -1;
            }
            if (init_sem(semid, val) == -1)
            {
                return -1;
            }
        }
        else
        {
            perror("Sem Get Error!\n");
            return -1;
        }
    }
    else
    {
        printf("sem have create\n");
        if (init_sem(semid, val) == -1)
        {
            printf("sem init fail\n");
            return -1;
        }
    }
    return semid;
}

int new_create_sem(key_t key, int val, int & createdMark)
{
    int semid;
    createdMark = 0;
    semid = semget(key, 1, 0600 | IPC_CREAT | IPC_EXCL);  //不存在则创建，存在返回错误eexist
    if(-1 == semid)
    {
        if(errno == EEXIST)
            semid = semget(key, 1, 0600 | IPC_CREAT);
        if(-1 == semid)
        {
            zprintf1("semget key %d error!\n", key);
            return -1;
        }
    }
    else
    {
        createdMark = 1;
    }

    if (createdMark && val >= 0)
    {
        semun init_op;
        init_op.val = val;
        if (-1 == semctl(semid, 0, SETVAL, init_op))
        {
            zprintf1("init sem %d error!\n", semid);
            perror("Sem init");
            return -1;
        }
    }
    return 0;
}

// 将信号量sem_id设置为init_value
int init_sem(int sem_id, int init_value)
{
    union semun sem_union;
    sem_union.val = init_value;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1)
    {
        perror("Sem init");
        return -1;
    }
    return 0;
}

// 删除sem_id信号量
int del_sem(int sem_id)
{
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
    {
        perror("Sem delete");
        return -1;
    }
    return 0;
}

// 对sem_id执行p操作
int sem_p(int sem_id)
{
    struct sembuf sem_buf;
    sem_buf.sem_num = 0;  //信号量编号
    sem_buf.sem_op  = -1; // P操作
    sem_buf.sem_flg = SEM_UNDO;  //系统退出前未释放信号量，系统自动释放

    if (semop(sem_id, &sem_buf, 1) == -1)
    {
        perror("Sem P operation");
        return -1;
    }
    return 0;
}

// 对sem_id执行p操作
int sem_p(int sem_id, int milliseconds)
{
    struct sembuf   sem_buf;
    struct timespec timeout;
    //    int ret;
    sem_buf.sem_num = 0;  //信号量编号
    sem_buf.sem_op  = -1; // P操作
    sem_buf.sem_flg = SEM_UNDO;  //系统退出前未释放信号量，系统自动释放

    timeout.tv_sec  = (milliseconds / 1000);
    timeout.tv_nsec = (milliseconds - timeout.tv_sec * 1000L) * 1000000L;

    return (semtimedop(sem_id, &sem_buf, 1, &timeout));
}

// 对sem_id执行V操作
int sem_v(int sem_id)
{
    struct sembuf sem_buf;
    sem_buf.sem_num = 0;
    sem_buf.sem_op  = 1; // V操作
    sem_buf.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_buf, 1) == -1)
    {
        perror("Sem V operation");
        return -1;
    }
    return 0;
}


int get_sem_count(int sem_id)
{
    union semun sem_union;

    return semctl(sem_id,0, GETVAL, sem_union);

}
