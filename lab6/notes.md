# Lab 6 – Inside the xv6 File System

Nicolae Turcanu, FAF-242. The answers to the "Observe" boxes.
The full report, with the commands and outputs behind every answer, is `Lab6_NicolaeTurcanu_FAF-242.docx`.

## Part 1. Read what xv6 shows you

1. **What do types 1, 2 and 3 mean (kernel/stat.h)?**

   Type 1 = T_DIR, a directory; type 2 = T_FILE, a regular file; type 3 = T_DEVICE, a device (here only console, size 0, because its data does not come from the disk).

2. **Which inode number does the root directory have, and why are . and .. the same there?**

   The root directory has inode 1 (ROOTINO; inode 0 is never used, so that inum == 0 can mark a free directory slot). . and .. are the same in / because the root has no parent: by convention its .. points to itself, so both entries hold inode 1 (.  1 1 1024 and ..  1 1 1024).

3. **An empty directory a has size 32. A directory entry has a fixed size. How big is one entry, and which two entries does a contain?**

   A directory is a table of struct dirent entries: a 2-byte inode number (ushort inum) plus a 14-byte name (DIRSIZ), so one entry is 16 bytes. The empty directory a (size 32) contains exactly two entries, 2 × 16 = 32: . (inode 28, a itself) and .. (inode 1, the root).

## Part 2. Write mystat

1. **Paste the outputs after ln and after rm. What changed and what did not?**

   After ln README r2: README: file, inode 2, links 2, size 2441 bytes and the same line for r2. After rm README: r2: file, inode 2, links 1, size 2441 bytes, and cat r2 | wc still prints 48 336 2441. Changed: only the link count (1 → 2 → 1) and the set of names – mystat README now fails. Not changed: the inode number (2), the type, the size and the content: both names are entries pointing to the same inode, so there was never a copy.

2. **Find the line in kernel/sysfile.c (sys_unlink) that decrements the link count. When are the data blocks freed?**

   In sys_unlink the line is ip->nlink--; at line 244 of kernel/sysfile.c (line 239, dp->nlink--, is the parent’s count when a directory is removed). The data blocks are not freed there: iunlockput(ip) calls iput(), which frees them only when this is the last in-memory reference to the inode and its link count is 0 (ip->ref == 1 && ip->nlink == 0). Then itrunc() (kernel/fs.c line 365) frees the data blocks and ifree() (line 377) marks the inode as free. So the blocks go when the last name is gone and no process has the file open.

3. **xv6 has no symbolic links and no permissions. Look at struct dinode in kernel/fs.h: which fields does an xv6 inode have?**

   struct dinode has only: type, major, minor (device numbers, used for T_DEVICE), nlink, size, and addrs[NDIRECT + 1] – 12 direct block numbers and one indirect block number. There is no owner, no permission bits, no timestamps and no symbolic-link type, which is why xv6 has no permissions and no ln -s. The structure is 2 + 2 + 2 + 2 + 4 + 13 × 4 = 64 bytes.

## Part 3. Measure the limits

1. **What does ls show for averyveryverylongname, and which constant explains it?**

   ls shows averyveryveryl – the first 14 characters of averyveryverylongname. The constant is DIRSIZ = 14: the name field of a directory entry has 14 bytes and the rest of the name is cut off when the entry is written. Lookups compare at most 14 characters (namecmp uses strncmp(s, t, DIRSIZ)), so both mystat averyveryverylongname and mystat averyveryveryl find the same inode 28. (A name of exactly 14 bytes has no terminating zero, which is why ls and my find copy it into a buffer and add one.)

2. **Compute the largest file xv6 can hold: 12 direct blocks plus one indirect block that holds block numbers of 4 bytes each, blocks of BSIZE bytes. Show the calculation.**

   One indirect block holds NINDIRECT = BSIZE / sizeof(uint) = 1024 / 4 = 256 block numbers. The largest file therefore has MAXFILE = NDIRECT + NINDIRECT = 12 + 256 = 268 blocks, and 268 × 1024 = 274 432 bytes = 268 KiB.

3. **From the nmeta line: how many blocks does the whole disk have, and how many are for the log, the inodes, and the data?**

   nmeta 47 (boot, super, log blocks 31, inode blocks 13, bitmap blocks 1) blocks 1953 total 2000: the whole disk has 2000 blocks (FSSIZE) of 1 KiB. Of these, 47 are metadata: 1 boot block, 1 superblock, 31 for the log (30 data blocks = LOGBLOCKS plus a header), 13 for the inodes (200 inodes × 64 bytes / 1024 = 12.5, rounded up) and 1 for the free-block bitmap. The remaining 1953 blocks are for data.
