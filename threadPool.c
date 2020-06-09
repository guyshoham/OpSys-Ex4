// Guy Shoham 302288444

#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include "threadPool.h"
static void* runThread(void* pool);

ThreadPool* tpCreate(int numOfThreads) {
  //allocate memory for ThreadPool
  ThreadPool* tp = (ThreadPool*) malloc(sizeof(ThreadPool));
  if (tp == NULL)
    return NULL;

  tp->queue = osCreateQueue();
  tp->poolSize = numOfThreads;
  tp->taskCount = 0;
  tp->isDestroy = 0;
  tp->threadsRunning = 0;
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
    tp->threadsRunning++;
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
  pthread_mutex_lock(&(threadPool->lock));

  //insert task to queue
  osEnqueue(threadPool->queue, task);
  threadPool->taskCount++;

  //unlock pool
  pthread_mutex_unlock(&(threadPool->lock));

  pthread_cond_signal(&threadPool->cond);

  return 0;
}
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
  threadPool->isDestroy = 1;
  threadPool->waitForTasks = shouldWaitForTasks;

  /* Wake up all worker threads */
  pthread_cond_broadcast(&(threadPool->cond));

  int i;
  /* Join all worker thread */
  for (i = 0; i <= threadPool->poolSize; i++) {
    pthread_join(threadPool->threads[i], NULL);
  }

  osDestroyQueue(threadPool->queue);

  pthread_cond_destroy(&threadPool->cond);

  free(threadPool->threads);

  free(threadPool);
}

static void* runThread(void* pool) {
  ThreadPool* tp = (ThreadPool*) pool;
  Task* task;

  while (!tp->isDestroy) {
    pthread_mutex_lock(&(tp->lock));

    pthread_cond_wait(&tp->cond, &tp->lock);

    if (!osIsQueueEmpty(tp->queue)) {

      task = osDequeue(tp->queue);
      tp->taskCount--;
      pthread_mutex_unlock(&(tp->lock));

      (*(task->func))(task->param);

      free(task);
    }
  }

  // after destroy is called

  while (!osIsQueueEmpty(tp->queue)) {
    task = osDequeue(tp->queue);
    tp->taskCount--;
    pthread_mutex_unlock(&(tp->lock));


    if (tp->waitForTasks) { //execute only if waitForTasks=true
      (*(task->func))(task->param);
    }

    free(task);

    pthread_mutex_lock(&(tp->lock));
  }

  pthread_mutex_unlock(&(tp->lock));

  tp->threadsRunning--;
}
