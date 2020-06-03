// Guy Shoham 302288444

#include <stddef.h>
#include <stdlib.h>
#include "threadPool.h"

ThreadPool* tpCreate(int numOfThreads) {
  //allocate memory for ThreadPool
  ThreadPool* tp = (ThreadPool*) malloc(sizeof(ThreadPool));
  if (tp == NULL)
    return NULL;

  tp->queue = osCreateQueue();

  tp->numOfThreads = numOfThreads;
  tp->activeThreads = 0;

  //TODO: start running the thread pool (no busy waiting)

  return tp;
}
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {

  if (shouldWaitForTasks){
    while (!osIsQueueEmpty(threadPool->queue)){
      //TODO: wait (no busy waiting)
    }
  }
  osDestroyQueue(threadPool->queue);

  free(threadPool);
}
int tpInsertTask(ThreadPool* threadPool, void (* computeFunc)(void*), void* param) {

  if (threadPool->activeThreads >= threadPool->numOfThreads) {
    return -1; //no available threads in thread pool
  }

  //allocate memory for Task
  Task* task = (Task*) malloc(sizeof(Task));
  task->func = computeFunc;
  task->param = param;

  //insert task to queue
  osEnqueue(threadPool->queue, task);
  threadPool->activeThreads++;

  return 0;
}

