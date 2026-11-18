#include "kernel/types.h"
#include "user/user.h"

// prime sieve with a chain of processes: each stage keeps one prime
// and passes on only the numbers that are not divisible by it

void stage(int in) __attribute__((noreturn));

void
stage(int in)
{
  int p, n;

  if(read(in, &p, sizeof(p)) != sizeof(p)){   // no numbers left: end of the chain
    close(in);
    exit(0);
  }
  printf("prime %d\n", p);

  int fd[2];
  pipe(fd);
  if(fork() == 0){
    close(fd[1]);                    // the next stage only reads
    close(in);                       // and does not need our input
    stage(fd[0]);
  }
  close(fd[0]);                      // this stage only writes to the next one
  while(read(in, &n, sizeof(n)) == sizeof(n))
    if(n % p != 0)
      write(fd[1], &n, sizeof(n));
  close(in);
  close(fd[1]);                      // end-of-file for the next stage
  wait(0);                           // wait for the whole rest of the chain
  exit(0);
}

int
main(int argc, char *argv[])
{
  int fd[2];
  pipe(fd);
  if(fork() == 0){
    close(fd[1]);
    stage(fd[0]);
  }
  close(fd[0]);
  for(int i = 2; i <= 35; i++)
    write(fd[1], &i, sizeof(i));
  close(fd[1]);
  wait(0);
  exit(0);
}
