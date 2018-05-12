#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "threadpool.h"

//主函数功能描述：
//创建一个线程池,初始化最大线程3， 添加10个任务

void* run(void* arg)
{
  int id = *(int*)arg;
  free(arg);
  printf("%#lX thread running ! id = %d\n", pthread_self(), id);
  sleep(1);
}

int main()
{
  threadpool_t pool;
  threadpool_init(&pool, 3);
  int i = 0;
  for(; i < 10; i++)
  {
    int *p = (int*)malloc(sizeof(int));
    *p = i;
    threadpool_add(&pool, run, (void*)p);
  }


  threadpool_destroy(&pool);
  sleep(15);
}
