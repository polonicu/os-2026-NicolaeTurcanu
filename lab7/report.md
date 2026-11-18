# Lab 7 – Three Small Programs

Nicolae Turcanu, FAF-242. The answers to the "Observe" boxes.
The full report, with the commands and outputs behind every answer, is `Lab7_NicolaeTurcanu_FAF-242.docx`.

## Part 1. fork + exec + wait

1. **The line perror("exec") is in the child, right after execlp. Why is it printed only with nosuchprog?**

   Because a successful exec never returns. It replaces the whole program of the child – code, data, stack – with ls (or date), so the line after execlp no longer exists in that process. Only when exec fails – nosuchprog is not found in any PATH directory, error ENOENT – does execlp return −1, and the next line, perror("exec"), runs and prints exec: No such file or directory.

2. **Which number does the child print as "my parent", and whose PID is it?**

   The child prints my parent is 929378 (Figure 1.1), and that is the PID of the parent process ./run – the same number it printed in its first line, parent: my PID is 929378. getppid() returns the PID of the process that called fork.

3. **What status does the parent see with ls, and what status with nosuchprog?**

   With ls (and with date) the parent sees status 0: the program ran and ended successfully. With nosuchprog it sees status 127: the child itself called exit(127) after the failed exec – the same code the shell uses for “command not found”.

## Part 2. A zombie, on purpose

1. **Which letter does ls have in the STAT column, and what does it mean?**

   The [ls] <defunct> line has the letter Z in the STAT column: a zombie. ls finished within milliseconds, but its parent (./zombie, state S, sleeping, and its PPID column shows it is the parent) has not called wait yet. The kernel has freed the memory of ls but keeps its entry in the process table – PID and exit status – until the parent collects it.

2. **After 20 seconds, is the [ls] <defunct> line still there? Which line of zombie.c made it disappear?**

   No, after 20 seconds the line is gone (Figure 2.2). It disappeared when the parent woke up and executed waitpid(pid, &status, 0); (line 25 of zombie.c): waitpid reads the exit status of the child and only then does the kernel remove the zombie’s entry. Right after that the parent printed its message and ended too, so bash reported [1]+ Done ./zombie > /dev/null.

## Part 3. Redirection: ls -l > out.txt

1. **Which number did open give, and why exactly that one?**

   open gave descriptor 1. The kernel always returns the lowest-numbered free descriptor. In the child, 0 (standard input) and 2 (standard error) were still open, and 1 had just been closed, so 1 was the smallest free number.

2. **ls was not changed at all. Why does its listing not appear on the screen?**

   Because ls writes to descriptor 1 as always, and exec keeps the open descriptors of the process: descriptor 1 now refers to out.txt, not to the terminal. ls does not know or care what 1 is. (That is also why out.txt appears in its own listing with size 0: the file was created and truncated before ls started, and ls writes into it only after it has listed the directory.)

3. **To redirect the errors instead (like 2> err.txt), which descriptor would you close before open?**

   Descriptor 2: close(2) and then open("err.txt", …) would return 2, because 0 and 1 would still be in use – exactly what the shell does for 2> err.txt.

## Part 4. A pipe: ls /etc | wc -l

1. **Did ./pipeline and ls /etc | wc -l give the same number?**

   Yes: ./pipeline printed 309, and ls /etc | wc -l in the shell also printed 309 (Figure 4.1).

2. **After the change, which of the two children never ends, ls or wc?**

   wc never ends. In Figure 4.2 ls is no longer in the table – it wrote its 309 lines, exited and was collected by the parent’s first wait – while wc -l is still there, sleeping in anon_pipe_read, and the parent sleeps in do_wait waiting for it. Ctrl-C (SIGINT to the group) ended both: [1]+ Interrupt ./pipeline_broken.

3. **Why, in one sentence: what is wc waiting for?**

   wc is waiting for end-of-file on the pipe, which a reader gets only when every write end of the pipe is closed – and the parent still holds p[1] open, so wc waits forever while the parent waits for wc.
