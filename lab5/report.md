# Lab 5 – The File System with Your Own Hands

Nicolae Turcanu, FAF-242. The answers to the "Observe" boxes.
The full report, with the commands and outputs behind every answer, is `Lab5_NicolaeTurcanu_FAF-242.docx`.

## Part 1. The tree and the file types

1. **For each path in the second line, write its type from the first letter of ls -l.**

   /etc – d, directory; /etc/passwd – -, regular file; /bin – l, symbolic link; /dev/null – c, character device (major 1, minor 3); /tmp – d, directory (its permissions end in t, see Part 3).

2. **What is /bin on your system: a directory or a link, and to where?**

   /bin is a symbolic link to usr/bin, i.e. to /usr/bin (the target is relative to /). file /bin confirms it: “symbolic link to usr/bin”. Fedora has merged /bin into /usr.

3. **Find one character device and one block device in /dev.**

   Character device: /dev/null (also autofs, btrfs-control, lines starting with c). Block device: /dev/sda, major 8 minor 0, the whole SSD, and its partitions /dev/sda1, /dev/sda2 (lines starting with b). Devices show “major, minor” numbers instead of a size.

4. **Which is bigger on your laptop, /var/log or /usr, and by roughly how much?**

   /usr is much bigger: 20 GiB against 2.3 GiB for /var/log, about 9 times (≈ 18 GiB more). /usr holds all installed programs and libraries; 2.3 GiB of logs is mostly the systemd journal.

## Part 2. Inodes and links

1. **Which number in stat is the inode, and which tells how many names the file has?**

   In stat, Inode: 19849400 is the inode number (also the first column of ls -li), and Links: 1 is the number of names (hard links) the file has.

2. **After rm notes.txt, why could you still read hard.txt? What does rm really remove?**

   Because hard.txt is not a copy but a second directory entry pointing to the same inode. rm does not delete the file: it removes one name (the system call is unlink) and decreases the link count. The inode and its data blocks are freed only when the count reaches 0 and no process has the file open (see Stretch c).

3. **Why did cat soft.txt fail at the end, although soft.txt still exists? How big is soft.txt and why exactly that size?**

   A symbolic link is a separate small file whose content is a path, here hard.txt. cat follows that path, and after rm hard.txt nothing exists under that name, so it reports “No such file or directory” although the link itself is still listed (a dangling link). Its size is 8 bytes: exactly the length of the text hard.txt (8 characters, no terminating zero).

4. **A new directory has 2 links, and 4 after you create two subdirectories. Explain where each link comes from.**

   Classic counting (seen on tmpfs): one link is the name d in the parent directory, the second is the entry . inside d itself; each subdirectory adds one more, its .. entry pointing back to d, so 2 + 2 = 4. The same four names exist on btrfs (d, d/., d/x/.., d/y/.., all inode 19849492); btrfs just reports 1 instead of counting them.

5. **Why does a 2-byte file use 4.0K, and a 1 GB file use 0?**

   Space is allocated in whole blocks; the block size here is 4096 bytes, so the 2 bytes of x\n occupy one 4 KiB block (stat shows Blocks: 8, counted in 512-byte units). truncate -s 1G only writes the size 1 GiB into the inode and allocates no blocks: it is a sparse file whose “holes” read as zeros, and du counts allocated blocks, so it shows 0.

## Part 3. Permissions

1. **Write 600, 644, 755 and 700 as rwx strings, and give one real use for each.**

   600 = rw-------: private files, e.g. an SSH private key ~/.ssh/id_ed25519. 644 = rw-r--r--: ordinary files everyone may read, e.g. /etc/passwd or a web page. 755 = rwxr-xr-x: programs and shared directories, e.g. /usr/bin/ls or my report.sh. 700 = rwx------: a private directory or script, e.g. ~/.ssh.

2. **With chmod 000 you own the file but cannot read it. Why does ownership not help?**

   The kernel picks exactly one class – owner, group or others – and checks only its bits. For the owner it uses the owner bits, which are ---, and does not fall back to the group or other bits. Ownership gives something else: the right to change the permissions, which is why chmod u+r worked without root and then cat printed secret.

3. **What is your umask, and how does it explain the permissions of new.txt and newdir?**

   My umask is 0022: it removes write permission for group and others from new files. A file is created with 666 → 666 & ~022 = 644 (-rw-r--r--, new.txt); a directory with 777 → 755 (drwxr-xr-x, newdir). Both match Figure 3.1.

4. **Explain the box experiment: what does r allow on a directory, and what does x allow?**

   r on a directory allows reading its list of names: with r only, ls printed f, but nothing could be done with it (ls -l shows ?????????, cat was denied). x allows using a name to reach the inode behind it (search / traverse): with x only, the list could not be read (ls: cannot open directory), but cat box/f worked because the name was already known.

5. **What do the s in /usr/bin/passwd and the t in /tmp mean, and why does passwd need the s?**

   The s in place of the owner’s x in -rwsr-xr-x is the setuid bit: the program runs with the identity of the file’s owner, root, instead of the user who started it. passwd needs it because it must write the new password hash into /etc/shadow, which belongs to root and has mode ----------: a normal user cannot even read it (cat: /etc/shadow: Permission denied). The t in drwxrwxrwt on /tmp is the sticky bit: everyone may create files in /tmp, but only the owner of a file (or of the directory, or root) may delete or rename it.

## Part 4. Disks and the kernel as files

1. **Draw your disk: disk name, partitions, which one is mounted at / and with what file system type.**

   One SSD, /dev/sda, 238.5 GiB, with a GPT partition table and three partitions (Figure 4.3): sda1 600 MiB vfat (FAT32) mounted on /boot/efi (the EFI system partition), sda2 1 GiB ext4 on /boot (kernels), and sda3 236.9 GiB btrfs, which is mounted on / (subvolume /root) and also on /home (subvolume /home). Swap is zram0, a compressed 8 GiB area in RAM, not a disk partition.

2. **How full is your root file system?**

   Almost completely full: /dev/sda3 is 237G, 230G used, only 4.4G available – 99%. (Space should be freed soon; btrfs behaves badly when it runs out of space.)

3. **How many CPU cores and how much RAM does your laptop have, according to /proc?**

   /proc/cpuinfo lists 8 processors: the Intel Core i7-6820HQ has 4 physical cores with Hyper-Threading, so the kernel sees 8 logical CPUs. MemTotal: 16248696 kB ≈ 15.5 GiB of RAM (16 GB installed; the rest is reserved by the firmware and the kernel).

4. **/proc/self/status showed the name of a program. Which one, and why that one?**

   It showed Name: head. /proc/self is a link that the kernel resolves to the directory of whichever process is looking at it, and the process that opened /proc/self/status was head itself, not the shell. In the same way, ls -l /proc/self/fd showed the descriptors of ls: 0, 1 and 2 on the terminal /dev/pts/1, and 3 on the /proc/<pid>/fd directory it was reading.

## Stretch (two of the optional tasks)

### (a) Magic numbers

xxd is not installed on this laptop (type xxd → not found; installing it needs root), so the same first line of bytes was shown with hexdump -C. Although the file is called notes.txt, file reports gzip compressed data. It decides by the content, not by the name: the first two bytes 1f 8b are the gzip magic number (the next byte, 08, means the deflate method; gzip also stored the original name passwd). For comparison, one.txt contains only 78 0a (x and a newline), printable characters, so file calls it ASCII text.

### (c) Deleting a file that is still open

A 50 MiB bigfile was opened on descriptor 3 of the shell and then removed. The link in /proc/$$/fd says /home/nicu/os-lab5/bigfile (deleted), and following it (ls -lL) shows the inode still alive with a link count of 0 and all 52428800 bytes. The name is gone, but the inode and its blocks are kept while a process has the file open; the space is freed only when the last descriptor is closed, here by exec 3<&-, after which descriptor 3 disappears. This is the second condition from Part 2: a file dies when it has no names and no open descriptors.
