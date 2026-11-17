#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

static void
show(char *when)
{
  struct sysinfo info;

  if (sysinfo(&info) < 0) {
    fprintf(2, "sysinfo: failed\n");
    exit(1);
  }
  printf("%s: free memory %lu bytes (%lu pages), processes %lu\n",
         when, info.freemem, info.freemem / 4096, info.nproc);
}

int
main(void)
{
  show("start         ");

  sbrk(10 * 4096);                  // take 10 pages of memory
  show("after sbrk 10p");

  int pid = fork();                 // one more process
  if (pid == 0) {
    pause(10);
    exit(0);
  }
  show("after fork    ");
  wait(0);
  show("after wait    ");
  exit(0);
}
