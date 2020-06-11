// Guy Shoham 302288444

#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "threadPool.h"
static void* runThread(void* pool);

ThreadPool* tpCreate(int numOfThreads) {
  //allocate memory for ThreadPool
  ThreadPool* tp = (ThreadPool*) malloc(sizeof(ThreadPool));
  if (tp == NULL)
    exit(-1);

  tp->queue = osCreateQueue();
  tp->poolSize = numOfThreads;
  tp->taskCount = 0;
  tp->isDestroy = 0;
  tp->waitForTasks = 0;
  pthread_cond_init(&tp->cond, NULL);

  //allocate memory for threads
  tp->threads = (pthread_t*) malloc(numOfThreads * sizeof(pthread_t));
  if (tp->threads == NULL) {
    tpDestroy(tp, 0);
    exit(-1);
  }

  int i;
  for (i = 0; i < numOfThreads; i++) {
    if (pthread_create(&(tp->threads[i]), NULL, runThread, (void*) tp) != 0) {
      tpDestroy(tp, 0);
      exit(-1);
    }
  }

  return tp;
}
int tpInsertTask(ThreadPool* threadPool, void (* computeFunc)(void*), void* param) {
  //allocate memory for Task
  Task* task = (Task*) malloc(sizeof(Task));
  if (task == NULL) {
    tpDestroy(threadPool, 0);
    exit(-1);
  }

  task->func = computeFunc;
  task->param = param;

  //lock pool while adding task
  if (pthread_mutex_lock(&(threadPool->lock)) != 0) {
    tpDestroy(threadPool, 0);
    exit(-1);
  }

  //insert task to queue
  osEnqueue(threadPool->queue, task);
  threadPool->taskCount++;

  //unlock pool
  if (pthread_mutex_unlock(&(threadPool->lock)) != 0) {
    tpDestroy(threadPool, 0);
    exit(-1);
  }

  if (pthread_cond_signal(&threadPool->cond) != 0) {
    tpDestroy(threadPool, 0);
    exit(-1);
  }

  return 0;
}
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
  threadPool->isDestroy = 1;
  threadPool->waitForTasks = shouldWaitForTasks;

  /* Wake up all worker threads */
  if (pthread_cond_broadcast(&(threadPool->cond)) != 0) {
    exit(-1);
  }

  int i;
  /* Join all worker thread */
  for (i = 0; i < threadPool->poolSize; i++) {
    if (pthread_join(threadPool->threads[i], NULL) != 0) {
      exit(-1);
    }
  }

  osDestroyQueue(threadPool->queue);

  if (pthread_cond_destroy(&threadPool->cond) != 0) {
    exit(-1);
  }

  free(threadPool->threads);

  free(threadPool);
}

static void* runThread(void* pool) {
  ThreadPool* tp = (ThreadPool*) pool;
  Task* task;

  while (!tp->isDestroy) {
    if (pthread_mutex_lock(&(tp->lock)) != 0) {
      tpDestroy(tp, 0);
      exit(-1);
    }

    if (pthread_cond_wait(&tp->cond, &tp->lock) != 0) {
      tpDestroy(tp, 0);
      exit(-1);
    }

    if (!osIsQueueEmpty(tp->queue)) {

      task = osDequeue(tp->queue);
      tp->taskCount--;
      if (pthread_mutex_unlock(&(tp->lock)) != 0) {
        tpDestroy(tp, 0);
        exit(-1);
      }

      (*(task->func))(task->param);

      free(task);
    }
  }

  // after destroy is called

  while (!osIsQueueEmpty(tp->queue)) {
    task = osDequeue(tp->queue);
    tp->taskCount--;
    if (pthread_mutex_unlock(&(tp->lock)) != 0) {
      tpDestroy(tp, 0);
      exit(-1);
    }

    if (tp->waitForTasks) { //execute only if waitForTasks=true
      (*(task->func))(task->param);
    }

    free(task);

    if (pthread_mutex_lock(&(tp->lock)) != 0) {
      tpDestroy(tp, 0);
      exit(-1);
    }
  }

  if (pthread_mutex_unlock(&(tp->lock)) != 0) {
    tpDestroy(tp, 0);
    exit(-1);
  }
  return NULL;
}
