#ifndef __CONDITION_H__
#define __CONDITION_H__
#include <pthread.h>

typedef struct condition{
  pthread_mutex_t pmutex;
  pthread_cond_t pcond;
}condition_t;

int condition_init(condition_t* cond);
int condition_lock(condition_t* cond);
int condition_unlock(condition_t* cond);
int condition_wait(condition_t* cond);
int condition_timedwait(condition_t* cond, const struct timespec* abstime);   //超时等待
int condition_signal(condition_t* cond);    //唤醒一个线程
int condition_broadcast(condition_t* cond); //唤醒所有的线程
int condition_destroy(condition_t* cond);



#endif// __CONDITION_H__
