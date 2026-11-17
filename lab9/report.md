# Lab 9 – Watching System Calls with strace

Nicolae Turcanu, FAF-242. Fedora 41, strace 6.17, gcc 14.3.1.
The full report with all the outputs is `Lab9_NicolaeTurcanu_FAF-242.docx`.

## Part 1. See the calls of a real command

`strace -o ls.trace -f -e trace=openat,read,write,close ls -l` → 88 calls of these four kinds:
32 openat (12 failed, ENOENT for the coreutils.mo translations), 18 read, 34 close, 4 write.

**Which system call does ls use to print to the screen? To which descriptor does it write?**
`write`, on descriptor 1 (standard output, the terminal):
`write(1, "-rw-r--r--. 1 nicu nicu  4688 No"..., 52) = 52`.
One `write` per line (4 lines → 4 writes), because stdout is line-buffered when it is a terminal.

## Part 2. A program that calls the kernel directly

```
open returned descriptor 3
write sent 14 bytes
read got 14 bytes: hello, kernel
```
Under strace: `openat(AT_FDCWD, "note.txt", O_WRONLY|O_CREAT|O_TRUNC, 0644) = 3`,
`write(3, "hello, kernel\n", 14) = 14`, `close(3)`, `openat(..., O_RDONLY) = 3`,
`read(3, "hello, kernel\n", 63) = 14`, `close(3)`; and each `printf` became a `write(1, ...)`.

**The descriptor returned by the first openat is 3, not 0, 1 or 2. Why are those three already taken?**
Every process starts with 0 (stdin), 1 (stdout) and 2 (stderr) already open: the shell opened them on
the terminal and they are inherited through fork and exec (`ls -l /proc/self/fd` shows 0, 1, 2 →
`/dev/pts/6`). The kernel always returns the lowest free descriptor, so the first file gets 3.

## Part 3. Count one kind of call

`strace -c ls -l` → 348 system calls of 40 kinds (22 failed); most frequent epoll_ctl 42, mmap 39,
close 34, openat 32. The socket/connect/epoll calls are `ls -l` asking systemd-userdb (UNIX sockets in
`/run/systemd/userdb/`) for the user and group names.

Counting only `write` (`strace -c -e trace=write ./syscalls`): **4** calls when stdout is the terminal,
**2** when it is redirected to a file. 1 is always the write into note.txt; the 3 printf lines are
3 writes on a line-buffered terminal but 1 single write of 80 bytes when stdout is a fully buffered file.
