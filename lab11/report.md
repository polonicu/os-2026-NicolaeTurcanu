# Lab 11 – The Race and the Semaphore

Nicolae Turcanu, FAF-242. Fedora 41, gcc 14.3.1, 8 CPUs, compiled with `gcc -Wall -pthread` (no `-O2`).
The full report with all the outputs is `Lab11_NicolaeTurcanu_FAF-242.docx`.

## Part 1. See the race

```
$ ./race
counter = 1094977 (expected 2000000)
$ ./race
counter = 1051713 (expected 2000000)
$ ./race
counter = 1064579 (expected 2000000)
```

1. `counter++` is three CPU steps (seen with `objdump -d race`):
   **load** `mov counter,%rax` → **add** `add $0x1,%rax` → **store** `mov %rax,counter`.
2. Each thread has its own register but `counter` is shared. A loads 5, B loads 5, A adds and stores 6,
   B adds and stores 6: two additions, but the counter grew by one – B's store overwrote A's.
3. The overlaps depend on timing nobody controls (which cores, when each thread starts, when the
   scheduler interrupts it, when the cache line moves between cores), and that differs at every run.

## Part 2. Fix it: mutex, then semaphore

`./mutex` and `./sem`: `counter = 2000000 (expected 2000000)` every time.
Times: race 0.006 s (wrong), mutex 0.143 s, sem 0.143 s.

1. Yes – 2 000 000 in all runs of both (3 runs each, plus the timed runs and the runs under strace).
2. With `sem_init(&s, 0, 2)` both threads can be inside the critical section at once, so it protects
   nothing and the race is back: 1999387, 1998893, 1997693. A semaphore used as a lock must start at 1.
3. Every addition now also costs a lock and an unlock (atomic instructions that move the lock's cache
   line between cores), and when the lock is taken the thread sleeps in the kernel with `futex`
   (`strace -f -c`: 7250 futex calls for mutex, 5473 for sem, 2 for race). Only one thread runs at a time.

## Part 3. A semaphore as a signal

```
B: waiting for the message...
A: message written
B: I read "hello"
```
(B's line at once, then 1 second of nothing: `real 0m1.004s`.)

1. With start value 1, B reads `""` (empty) and prints it before A writes: `sem_wait` finds 1 unit and
   returns at once, while `message` is still all zeros (a global array).
2. A semaphore that starts at 0 means "the event has not happened yet": `sem_wait` blocks until
   another thread signals it with `sem_post`.

## Part 4. A semaphore as a counter: the parking lot

1. The largest value of `inside` is 3. It is guaranteed by `sem_wait(&places);` (with
   `sem_init(&places, 0, PLACES)`): the semaphore starts at 3, every car takes one unit before entering,
   the 4th finds 0 and waits until a car leaves and calls `sem_post`.
2. With `PLACES` = 1 the semaphore becomes a binary semaphore – a lock (mutex): one car at a time,
   `inside` never above 1, 6 seconds instead of 2.

## Optional: atomic

`atomic_fetch_add` compiles to one instruction, `lock addq $0x1,counter`.
Result 2 000 000 in 0.074 s, against 0.134 s for the mutex, and almost no `sys` time (no futex).
