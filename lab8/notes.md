# Lab 8 – The Same Calls Inside Your Own OS

Nicolae Turcanu, FAF-242. The answers to the "Observe" boxes.
The full report, with the commands and outputs behind every answer, is `Lab8_NicolaeTurcanu_FAF-242.docx`.

## Part 1. run, inside xv6

1. **Write the three calls that run.c uses in xv6 and the three calls run.c used on Linux in Lab 7. What is the same?**

   xv6: fork(), exec("echo", args), wait(&status). Linux (Lab 7): fork(), execlp("ls", "ls", "-l", NULL), waitpid(pid, &status, 0). The same: the three steps and what each call means – fork makes a copy and returns twice (0 in the child, the child’s PID in the parent), exec replaces the program of the calling process and returns only if it fails, and the parent blocks in wait until the child has ended and then gets its exit status. Only the calling details differ (Table 1.1). One more small difference: Lab 7 needed fflush(stdout) before fork, but xv6’s printf writes every character with write at once, so there is no buffer to flush.

2. **Who prints "hello from the child": the code of run, or echo?**

   echo prints it. Up to exec the child runs the code of run and prints child: my pid is 4. exec("echo", args) then replaces that code with the program /echo; run only chose the words in args, and echo printed them. The proof is that exec failed was not printed: the rest of run’s code was gone from the child.

3. **Why is the status 1 here and 127 on Linux? (Look at the exit line after exec in both programs.)**

   The status is simply the number the child passed to exit, and the parent receives it unchanged. In xv6’s run.c the line after a failed exec is exit(1);, so the parent saw 1; in Lab 7’s run.c it was exit(127);, so the parent saw 127. 127 is only the convention the shell uses for “command not found”; the kernel does not care which number is used.

## Part 2. pingpong

1. **Why do you need TWO pipes, and not one for both directions?**

   A pipe is a one-way channel: whatever is written into its write end can be read by any process that holds its read end, in order. With a single pipe for both directions, after the parent wrote the ping and then called read to wait for the pong, the parent itself could read its own ping byte back before the child got it, and the child would wait forever. There is no way to address a byte to a particular reader, so each direction needs its own pipe: p2c carries data only from parent to child, c2p only from child to parent.

2. **Nothing in your code says "wait until the child has replied". Which call makes the parent wait?**

   The parent’s read(c2p[0], &b, 1). Reading from an empty pipe blocks while some process still holds its write end open, so the parent sleeps in read until the child has written the byte back. The child’s read(p2c[0], &b, 1) blocks in the same way until the ping arrives. That is why “received ping” is always printed before “received pong”: the child writes the pong only after printing ping.
