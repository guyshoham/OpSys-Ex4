#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "osqueue.h"
#include "threadPool.h"

void hello(void* a) {
  sleep(1);
  printf("hello %d\n", (int) a);
}

void test_thread_pool_sanity() {
  int i;

  ThreadPool* tp = tpCreate(2);

  for (i = 1; i <= 4; ++i) {
    tpInsertTask(tp, hello, (void*) i);
  }
  sleep(1);

  tpDestroy(tp, 1);
  printf("done\n");
}

int main() {
  test_thread_pool_sanity();

  return 0;
}
