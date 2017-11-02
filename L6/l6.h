#ifndef L6_H
#define L6_H

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

extern MINODE minodes[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern MTABLE mtable[4]; 

extern SUPER *sp;
extern GD    *gp;
extern INODE *ip;

extern int dev;
extern int nblocks; // from superblock
extern int ninodes; // from superblock
extern int bmap;    // bmap block 
extern int imap;    // imap block 
extern int iblock;  // inodes begin block

// device
extern char *device = "mydisk";
// block data buff
extern char buff[BLKSIZE];
extern char buff2[BLKSIZE];
// dir names
extern char *names[64];

#endif