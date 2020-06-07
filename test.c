#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "osqueue.h"
#include "threadPool.h"

void hello(void* a) {
  sleep(5);
  printf("hello %d\n", (int) a);
}

void test_thread_pool_sanity() {
  int i;

  ThreadPool* tp = tpCreate(5);

  for (i = 1; i <= 10; ++i) {
    tpInsertTask(tp, hello, (void*) i);
  }
  sleep(2);

  tpDestroy(tp, 0);
}

int main() {
  test_thread_pool_sanity();

  return 0;
}
