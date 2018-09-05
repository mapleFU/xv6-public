// On-disk file system format.
// Both the kernel and user programs use this header file.


#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// Disk layout:
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
  uint logstart;     // Block number of first log block
  uint inodestart;   // Block number of first inode block
  uint bmapstart;    // Block number of first free map block
};

// will modify 12 * 1 + 128 to 11 * 1 + 128 + 128 * 128 == 16523
// direct blocks
#define NDIRECT 11
// second direct blocks: No 11(12) 
#define NSECDIR 12

// the number of the file to indir stored
#define NINDIRECT (BSIZE / sizeof(uint))
// sec per file numbers
#define NSEC (NINDIRECT * NINDIRECT)
#define MAXFILE (NDIRECT + NINDIRECT + NSEC)

// On-disk inode structure
// 64 bytes.
struct dinode {
  short type;           // File type --> 2
  short major;          // Major device number (T_DEV only) --> 2
  short minor;          // Minor device number (T_DEV only) --> 2
  short nlink;          // Number of links to inode in file system --> 2
  uint size;            // Size of file (bytes) --> 4
  uint addrs[(NSECDIR + 1)];   // Data block addresses --> 4 * 13 == 52
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

// the struct is 16 bytes.
struct dirent {
  ushort inum; 
  char name[DIRSIZ];
};

