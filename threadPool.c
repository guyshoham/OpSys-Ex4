// Guy Shoham 302288444

#include <pthread.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "threadPool.h"
static void* runThread(void* threadpool);

ThreadPool* tpCreate(int numOfThreads) {
  //allocate memory for ThreadPool
  ThreadPool* tp = (ThreadPool*) malloc(sizeof(ThreadPool));
  if (tp == NULL)
    return NULL;

  tp->queue = osCreateQueue();
  tp->poolSize = numOfThreads;
  tp->taskCount = 0;
  tp->shutdown = 0;
  tp->threadRunning = 0;
  tp->waitForTasks = 0;
  pthread_cond_init(&tp->cond, NULL);

  //allocate memory for threads
  tp->threads = (pthread_t*) malloc(numOfThreads * sizeof(pthread_t));
  if (tp->threads == NULL)
    return NULL;

  int i;
  for (i = 1; i <= numOfThreads; i++) {
    if (pthread_create(&(tp->threads[i]), NULL, runThread, (void*) tp) != 0) {
      tpDestroy(tp, 0);
      return NULL;
    }
    tp->threadRunning++;
  }

  return tp;
}
int tpInsertTask(ThreadPool* threadPool, void (* computeFunc)(void*), void* param) {

  //allocate memory for Task
  Task* task = (Task*) malloc(sizeof(Task));
  if (task == NULL)
    return -1;

  task->func = computeFunc;
  task->param = param;


  //lock pool while adding task
  //printf("lock tpInsertTask\n");
  pthread_mutex_lock(&(threadPool->lock));

  //insert task to queue
  osEnqueue(threadPool->queue, task);
  threadPool->taskCount++;

  //unlock pool
  //printf("unlock tpInsertTask\n");
  pthread_mutex_unlock(&(threadPool->lock));

  pthread_cond_signal(&threadPool->cond);

  return 0;
}
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
  printf("tpDestroy\n");
  threadPool->shutdown = 1;
  threadPool->waitForTasks = shouldWaitForTasks;

  /* Wake up all worker threads */
  pthread_cond_broadcast(&(threadPool->cond));
  printf("broadcast\n");

  int i;
  /* Join all worker thread */
  for (i = 1; i <= threadPool->poolSize; i++) {
    pthread_join(threadPool->threads[i - 1], NULL);
    printf("join: %d\n", i);
  }


  /*if (shouldWaitForTasks) { //wait for tasks in queue to terminate
    printf("shouldWaitForTasks: %d\n", threadPool->taskCount);
    while (!osIsQueueEmpty(threadPool->queue)) {

      printf("%d\n", osIsQueueEmpty(threadPool->queue));

      //printf("%d\n", threadPool->taskCount);
      //TODO: wait (no busy waiting)
    }
    printf("empty\n");

  }*/
  osDestroyQueue(threadPool->queue);

  free(threadPool->threads);
  free(threadPool);
}

static void* runThread(void* pool) {
  ThreadPool* tp = (ThreadPool*) pool;
  Task* task;

  while (!tp->shutdown) {
    pthread_mutex_lock(&(tp->lock));

    pthread_cond_wait(&tp->cond, &tp->lock);

    if (!osIsQueueEmpty(tp->queue)) {

      task = osDequeue(tp->queue);
      tp->taskCount--;
      printf("dequeue... %d\n", tp->taskCount);
      pthread_mutex_unlock(&(tp->lock));

      //printf("executing...\n");
      (*(task->func))(task->param);
    }
  }

  if (tp->waitForTasks) { // run all queue

    while (!osIsQueueEmpty(tp->queue)) {
      task = osDequeue(tp->queue);
      tp->taskCount--;
      printf("dequeue... %d\n", tp->taskCount);
      pthread_mutex_unlock(&(tp->lock));

      //printf("executing...\n");
      (*(task->func))(task->param);
      pthread_mutex_lock(&(tp->lock));
    }

  }

  pthread_mutex_unlock(&(tp->lock));

  tp->threadRunning--;

  printf("shutting down thread\n");
}


