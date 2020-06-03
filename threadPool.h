#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"

typedef struct task {
  void (* func)(void*);
  void* param;
} Task;

typedef struct thread_pool {
  int numOfThreads;
  int activeThreads;
  OSQueue* queue;

} ThreadPool;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (* computeFunc)(void*), void* param);

#endif
