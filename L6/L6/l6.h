#include <ext2fs/ext2_fs.h>
#include <stdio.h>
#include <fcntl.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

#define BLKSIZE     1024
#define SBLK           1
#define GDBLK          2

#define NMINODE      100
#define NFD           16
#define NPROC          2

typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  int mounted;
  struct mntTable *mptr;
}MINODE;

typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

typedef struct proc{
  struct proc *next;
  int          pid;
  int          uid;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

typedef struct mntTable{
  int dev;         // dev number: 0=FREE
  int nblock;      // s_blocks_count
  int ninodes;     // s_inodes_count
  int bmap;        // bmap block#
  int imap;        // imap block# 
  int iblock;      // inodes start block#
  MINODE *mountDirPtr;
  char deviceName[64];
  char mountedDirName[64];
}MTABLE;