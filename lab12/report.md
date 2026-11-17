# Lab 12 – Producer, Consumer and the Other Tools

Nicolae Turcanu, FAF-242. Fedora 41, gcc 14.3.1, 8 CPUs, `gcc -Wall -pthread`.
The full report with all the outputs is `Lab12_NicolaeTurcanu_FAF-242.docx`.
`deadlock.c` in this folder is the fixed version.

## Part 1. Producer and consumer

`./prodcons` gave exactly the expected output (put 1–4, then take/put alternating, then take 5–8), 2.4 s.

1. `sem_wait(&empty);` – `empty` starts at 4 (free slots); after 4 puts it is 0 and the producer blocks
   until the consumer gives a slot back with `sem_post(&empty)`.
2. The consumer waits, in `sem_wait(&full);` – `full` starts at 0, nothing to take until the first
   `sem_post(&full)` of the producer.
3. With the mutex locked before `sem_wait(&empty)`: put 1–4, then it hangs (`timeout 5` → exit code 124).
   Deadlock: the producer sleeps in `sem_wait(&empty)` while holding the mutex; only the consumer can free
   a slot, but it blocks in `pthread_mutex_lock` waiting for that mutex. Rule: `sem_wait` first, then lock.

## Part 2. The condition variable

`./condvar`: `B: not ready, I sleep` / `A: message written` / `B: I read "hello"` (1 s pause).

1. The variable `ready` (protected by the mutex). The condition variable has no memory; it is only the
   place where B sleeps.
2. Yes, with `if` it still works here: one reader, one writer, and A sets `ready = 1` before signalling.
3. A thread can wake up with `ready == 0`: spurious wakeups (POSIX allows `pthread_cond_wait` to return
   without a signal), or with several waiters / `broadcast` another thread gets the mutex first and
   consumes the condition. `while` re-checks after every wake-up.

## Part 3. Readers and writers

`./readers` (twice): writer 1 writes 10 alone; readers 1, 2, 3 read 10 together (inside: 1, 2, 3);
writer 2 writes 20 alone.

1. `if (++readers == 1) sem_wait(&room);` (only the first reader takes the room) and
   `if (--readers == 0) sem_post(&room);` (only the last reader gives it back).
2. `++readers`/`--readers` are read–add–write like `counter++`, and the test must go with the change.
   Without `rlock`: two readers can both think they are first (the second blocks on `room` although
   only readers are inside); a lost decrement keeps `readers` > 0 forever, so the room is never released
   and the writers wait forever; or two readers both see 0 and post twice, `room` becomes 2 and two
   writers can be inside together.
3. Writer 2 starves: while at least one reader is inside, new readers walk in without touching `room`;
   if the counter never drops to 0 the room is never released and writer 2 waits forever.

## Part 4. Build a deadlock, then remove it

Original: `one: I have A, I want B` / `two: I have B, I want A`, 5 s of nothing, `exit code: 124`
(`ps -L`: all threads asleep in `futex_do_wait`). Fixed (same order A then B in both): both "I have both",
`done`, `exit code: 0`.

1. one holds A and waits for B; two holds B and waits for A.

   ```
   one --holds--> A <--wants-- two
   one --wants--> B <--holds-- two
   circle: one -> B -> two -> A -> one
   ```
2. Without `sleep(1)`, ten runs gave exit codes `124 0 124 124 124 124 0 124 0 124`: it hangs only
   sometimes (7 of 10). Worse, because the bug is still there but random: it passes some tests, then
   freezes later, and it is hard to reproduce.
3. If all threads take the locks in the same order, whoever holds B already holds A, so no one can hold
   B while waiting for A – the circle can never close.
