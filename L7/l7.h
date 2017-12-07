#ifndef L7_H
#define L7_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

#define BLKSIZE     1024
#define SBLK           1  // superblock
#define GDBLK          2  // group descriptor

#define NMINODE      100  // inode in memory
#define NFD           16  // number file descriptor
#define NPROC          2  // number process

#define READ_MODE      0
#define WRITE_MODE     1
#define RW_MODE        2
#define APPEND_MODE    3

#define CMD_BUFF     128

typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;           // number of PROCs using this file
  int dirty;
  int mounted;            // on seconde inode
  int lock;               // 1 for lock, 0 for unlock
  struct mntTable *mptr;  // pointer of mount table
}MINODE;

typedef struct oft{
  int  mode;        // R/W/RW/APPEND/etc.
  int  refCount;    // number of PROCs using this file
  MINODE *mptr;     // pointer of MINODE
  int  offset;
}OFT;

typedef struct proc{
  struct proc *next;
  int          pid; // process
  int          uid; // owner uid
  MINODE      *cwd; // pointer of cwd on currend inode in memory
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
extern MTABLE mtable[4];  // 4 mount spot

extern SUPER *sp;   // superblock pointer
extern GD    *gp;   // group descriptor pointer
extern INODE *ip;   // inode pointer

extern int dev;     // device
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

// cmd args
extern char *myargv[64];

#endif


/*


struct ext2_super_block {
  u32 s_inodes_count;        // total number of inodes
  u32 s_blocks_count;        // total number of blocks
  u32 s_r_blocks_count;
  u32 s_free_blocks_count;   // current number of free blocks
  u32 s_free_inodes_count;   // current number of free inodes
  u32 s_first_data_block;    // first data block: 1 for FD, 0 for HD
  u32 s_log_block_size;      // 0 for 1KB block, 2 for 4KB block
  u32 s_log_frag_size;       // not used
  u32 s_blocks_per_group;    // number of blocks per group
  u32 s_frags_per_group;     // not used
  u32 s_inodes_per_group;
  u32 s_mtime, s_wtime;
  u16 s_mnt_count;           / number of times mounted
  u16 s_max_mnt_count;       // mount limit
  u16 s_magic;               // 0xEF53
                             // MORE non-essential fileds,
  u16 s_inode_size=256 bytes for EXT4
};


struct ext2_group_desc
{
  u32 bg_block_bitmap;      // Bmap block number
  u32 bg_inode_bitmap;      // Imap block number
  u32 bg_inode_table;       // Inodes begin block number
  u16 bg_free_blocks_count; // THESE are OBVIOUS
  u16 bg_free_inodes_count;
  u16 bg_used_dirs_count;
  u16 bg_pad;               // ignore these
  u32 bg_reserved[3];
};


struct ext2_inode {
  u16 i_mode;         // 16 bits = |tttt|ugs|rwx|rwx|rwx|, for tttt: REG 1000 | DIR 0100
  u16 i_uid;          // owner uid
  u32 i_size;         // file size in bytes
  u32 i_atime;        // time fields in seconds
  u32 i_ctime;        // since 00:00:00,1-1-1970
  u32 i_mtime;
  u32 i_dtime;
  u16 i_gid;          // group ID
  u16 i_links_count;  // hard-link count
  u32 i_blocks;       // number of 512-byte sectors
  u32 i_flags;        // IGNORE
  u32 i_reserved1;    // IGNORE
  u32 i_block[15];    // See details below
  u32 i_pad[7];
}


struct ext2_dir_entry_2 {
  u32 inode;                // inode number; count from 1, NOT 0
  u16 rec_len;              // this entry's length in bytes
  u8 name_len;              // name length in bytes
  u8 file_type;             // not used
  char name[EXT2_NAME_LEN]; // name: 1 -255 chars, no NULL byte
};
*/