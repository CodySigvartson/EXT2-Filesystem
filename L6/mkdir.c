#include "l6.h"

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

int my_mkdir_util(MINODE *pmip, char *dir){
    int ino = ialloc(dev);
    int blk = balloc(dev);

    // create the new inode
    MINODE *mip = iget(dev,ino); // load the newly allocated inode into mem
    INODE *ip = &mip->INODE;
    ip->i_mode = (unsigned short)0x41ED; // DIR type & permissions
    ip->i_uid = (unsigned short)running->uid; // owner uid
    ip->i_gid = (unsigned short)running->pid; // group id
    ip->i_size = BLKSIZE;
    ip->i_links_count = (unsigned short)2;
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 2;
    ip->i_block[0] = blk;
    for(int i = 1; i < 15;i++){
        ip->i_block[i] = 0;
    }
    mip->dirty = 1;
    iput(mip);

    // create the . and .. dirs for the new inode
    char buf[BLKSIZE];
    bzero(buf,BLKSIZE);
    DIR *dp = (DIR *)buf;
    // make the . entry
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';
    // make the .. entry
    dp = (char *)dp + 12;
    dp->inode = pmip->ino;
    dp->rec_len = BLKSIZE-12;
    dp->name_len = 2;
    dp->name[0] = dp->name[1] = '.';
    put_block(dev,blk,buf);

    // enter the newly created dir as a dir of the parent
    enter_child(pmip,ino,dir);
}

// makes a directory in the filesystem
int my_mkdir(char *pathname){
    if(!pathname)
        return -1;
    char parentPathCpy[128], dirPathCpy[128];
    strcpy(parentPathCpy,pathname);
    strcpy(dirPathCpy,pathname);

    char *parent = dirname(parentPathCpy);
    char *dir = basename(dirPathCpy);
    printf("mkdir(): parent: %s\n",parent);
    printf("mkdir(): dir: %s\n",dir);

    int pino;
    if(strcmp(parent,"/")==0){
        pino = 2;
    }
    else if(strcmp(parent,".")==0){
        pino = running->cwd->ino;
    }
    else{
        pino = getino(parent);
    }

    MINODE *pmip = iget(dev,pino);
    INODE *ip = &pmip->INODE;
    if(S_ISDIR(ip->i_mode)){
        int isFound = search(pmip,dir);
        if(isFound == -1){
            my_mkdir_util(pmip,dir);
        }
    }
    pmip->INODE.i_links_count++;
    pmip->dirty=1;
}