#include "l7.h"

MINODE minodes[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
MTABLE mtable[4]; 

SUPER *sp;
GD    *gp;
INODE *ip;

int dev;
int nblocks; // from superblock
int ninodes; // from superblock
int bmap;    // bmap block 
int imap;    // imap block 
int iblock;  // inodes begin block

// device
char *device;
// block data buff
char buff[BLKSIZE];
char buff2[BLKSIZE];
// dir names
char *names[64];

// change cwd to pathname
int cd(char *pathname){
    int ino;
    printf("cd(): changing directory...\n");
    if(!pathname){
        // cd to root
        ino = getino("/");
    }else{
        printf("cd(): getting ino...\n");
        ino = getino(pathname);
        printf("cd(): ino found at %d",ino, running->cwd->ino);
    }
    MINODE *mip = iget(dev,ino);
    if(S_ISDIR(mip->INODE.i_mode)){
        printf("cd(): inode is DIR...\n");
        iput(running->cwd);
        running->cwd = mip;
        printf("cd(): running->cwd changed\n");
        return ino;
    }
}