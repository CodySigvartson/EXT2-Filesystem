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

// pwd() utility function
int rpwd(MINODE *wd){
    printf("rpwd(): getting working directory...\n");
    int my_ino, par_ino;
    char dirname[64];
    printf("rpwd(): before root check\n");
    if(wd == root){
        return;
    }
    printf("rpwd(): after root check\n");
    INODE *tip = &wd->INODE;
    // get my_ino and parent_ino
    my_ino = wd->ino;
    printf("rpwd(): my_ino is %d\n",my_ino);
    int blk = tip->i_block[0];
    get_block(dev,blk,buff);
    // get to parent_ino location
    DIR *dp = (DIR *)buff;
    char *cp = buff;
    cp += dp->rec_len;
    dp = (DIR *)cp;

    par_ino = dp->inode;
    printf("rpwd(): par_ino is %d\n",par_ino);

    // get parent inode to get name of wd
    MINODE *pmip = iget(dev,par_ino);
    tip = &pmip->INODE;
    blk = tip->i_block[0];
    get_block(dev,blk,buff);
    dp = (DIR *)buff;
    cp = buff;
    printf("rpwd(): iterating dirs...\n");
    while(cp < buff + BLKSIZE){
        if(dp->inode == my_ino){
            strcpy(dirname,dp->name);
            printf("rpwd(): dirname is %s\n",dirname);
            break;
        }
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    printf("rpwd(): making recursive call\n");
    rpwd(pmip);
    printf("/%s\n",dirname);
}

// prints the working directory
int pwd(MINODE *wd){
    if(wd == root){
        printf("current working directory: /\n");
    }
    else{
        rpwd(wd);
    }
}