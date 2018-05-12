#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "threadpool.h"
//线程池功能描述：
//当任务增加时，可以动态增加线程池中线程的个数，
//当任务执行完成后，可以动态的减少线程池中线程的个数。

//生产者线程向任务队列中添加任务，任务队列中有任务，如果有等待线程就唤醒并执行任务，
//如果线程池中没有等待线程并且没有达到上限，就添加新的线程到线程池


//线程执行的任务函数
void* thread_fun(void* arg)   
{
  threadpool_t* pool = (threadpool_t*)arg;
  int timeout = 0;
  while(1)    //线程没有任务也不退出
  {
    condition_lock(&pool->cond);
    pool->idle++;   //线程没有执行任务，先置为空闲线程
    timeout = 0;
    while(pool->first == NULL && pool->quit == 0)
    {
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);   //获取当前的绝对时间。
      ts.tv_sec += 2;         
      int r = condition_timedwait(&pool->cond, &ts);
      if(r == ETIMEDOUT)
      {
        timeout = 1;
        break;
      }
    }
    pool->idle--;
    
    if(NULL != pool->first)
    {
      task_t *p_first = pool->first;      //取出队头结点
      pool->first = p_first->_next;

      condition_unlock(&pool->cond);  //防止调用的任务执行时间过长，导致别的线程无法进入22行的lock,甚至于往线程池添加任务的锁都无法进入
      //这里的解锁不会影响对链表的操作,因为这里将链表中的单个结点拿出来进行操作，并没有涉及到对链表的操作(没有操作线程之间共享的数据)
      (p_first->pfun)(p_first->arg);  //调用pool传递进来的任务函数  
      condition_lock(&pool->cond);

      free(p_first);
      p_first = NULL;
    }
    
    if(NULL == pool->first && 1 == timeout) //防止其他线程添加任务，这时应该去执行相应的任务，
    {
      //此时确定是线程2秒内没有任务，导致的退出
      condition_unlock(&pool->cond);
      printf("%#lX thread TIMEOUT!  QUIT!!!\n", pthread_self());
      break;
    }

    if(pool->quit == 1 && NULL == pool->first)
    {
      printf("%#lX thread destroy!\n", pthread_self());
      pool->counter--;
      if(0 == pool->counter)
      {
        condition_signal(&pool->cond);  //这里发送信号提示最后一个线程任务结束
      }
      condition_unlock(&pool->cond);
      break;
    }
    condition_unlock(&pool->cond);
  }
}

void threadpool_init(threadpool_t* pool, int max)
{
  assert(NULL != pool);
  condition_init(&pool->cond);
  pool->first      = NULL;
  pool->tail       = NULL;
  pool->max_thread = max;
  pool->idle       = 0;
  pool->counter    = 0;
  pool->quit       = 0;
}
//往线程池添加线程
void threadpool_add(threadpool_t* pool, void*(*pf)(void*), void* arg)
{
  //先将任务生成任务节点
  task_t* new_task = (task_t*)malloc(sizeof(task_t));
  new_task->pfun   = pf;
  new_task->arg    = arg;
  new_task->_next  = NULL;
  
  //由于队列的插入操作、及线程池线程的调用操作是不能被打断的
  //所以以下动作需加锁
  condition_lock(&pool->cond);
  //将任务插入任务队列等待调度
  
  if(NULL == pool->first)       
  {
    pool->first = new_task;
  }
  else
  {
    pool->tail->_next = new_task;
  }

  pool->tail  = new_task;     //跳整队尾指针
  //当线程池中存在空闲线程时,直接唤醒空闲线程
  if(pool->idle > 0)      //有空闲线程
  {
    condition_signal(&pool->cond);
  }
  //当线程池中没有空闲线程且线程总数没有超出最大线程，则创建线程并执行
  else if(pool->counter < pool->max_thread)
  {
    pthread_t tid;
    pthread_create(&tid, NULL, thread_fun, (void*)pool);
    pool->counter++;
  }

    condition_unlock(&pool->cond);
}

void threadpool_destroy(threadpool_t* pool)
{
  if(pool->quit) return;

  condition_lock(&pool->cond);
  
  pool->quit = 1;     //将所有的线程状态置为退出态
  if(pool->counter > 0)
  {
    if(pool->idle > 0)
    condition_broadcast(&pool->cond);
  }
  while(pool->counter > 0)
  {
    condition_wait(&pool->cond);
  }
  condition_unlock(&pool->cond);
  condition_destroy(&pool->cond);
}


