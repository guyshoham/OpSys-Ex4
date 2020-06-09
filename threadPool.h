#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"

typedef struct task {
  void (* func)(void*);
  void* param;
} Task;

typedef struct thread_pool {
  int poolSize;
  int taskCount;
  int threadsRunning;
  int isDestroy;
  int waitForTasks;
  OSQueue* queue;
  pthread_t* threads;
  pthread_mutex_t lock;
  pthread_cond_t cond;

} ThreadPool;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (* computeFunc)(void*), void* param);

#endif
