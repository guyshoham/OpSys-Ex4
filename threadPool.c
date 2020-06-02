//
// Created by guy on 02/06/2020.
//

#include <stddef.h>
#include "threadPool.h"

ThreadPool* tpCreate(int numOfThreads) {
  return NULL;
}
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {

}
int tpInsertTask(ThreadPool* threadPool, void (* computeFunc)(void*), void* param) {
  return 0;
}

