#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "condition.h"

typedef struct task{        //队列结点的结构体
    void* (*pfun)(void*);   //任务队列的回调函数
    void* arg;              //回调函数的参数
    struct task* _next;     //链表指针
}task_t;

typedef struct threadpool{
    condition_t cond;     //同步、互斥操作
    task_t* first;        //任务队列的队头
    task_t* tail;         //任务队列的队尾
    int max_thread;       //最大线程数
    int idle;             //空闲线程的个数
    int counter;          //线程池当前的线程个数
    int quit;          //为1表示退出，为0表示不退出
}threadpool_t;

//初始化
void threadpool_init(threadpool_t* pool, int max);

//往线程池中添加任务
void threadpool_add(threadpool_t* pool, void*(*pf)(void*), void* arg);

//销毁线程池(动态的销毁线程池---当一定时间内，某线程一直没有被调用过，认为可以将该线程销毁)
void threadpool_destroy(threadpool_t* pool);



#endif// __THREADPOOL_H__
