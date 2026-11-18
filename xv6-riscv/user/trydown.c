// trydown.c: sem_trydown takes a unit if there is one, else returns -1 at once.
#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  sem_init(4, 1);                        // semaphore 4 starts at 1
  printf("value 1: sem_trydown -> %d (took the unit)\n", sem_trydown(4));
  printf("value 0: sem_trydown -> %d (no unit, did not sleep)\n", sem_trydown(4));
  sem_up(4);                             // give one back
  printf("after sem_up: sem_trydown -> %d\n", sem_trydown(4));
  exit(0);
}
