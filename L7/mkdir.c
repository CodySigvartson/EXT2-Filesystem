#include "l7.h"

/////////////////////////////////////////////////////////////////////////
// my_mkdir_util()
// creates the new dir with proper permissions
// pmip: parent inode to add new dir as child of
// dir: name of new dir to create
/////////////////////////////////////////////////////////////////////////
int my_mkdir_util(MINODE *pmip, char *dir){
    // allocate a new block and inode location on disk
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
    // write these dirs into the new dir's data blocks
    put_block(dev,blk,buf);

    // enter the newly created dir as a dir of the parent
    enter_child(pmip,ino,dir);
}

/////////////////////////////////////////////////////////////////////////
// my_mkdir()
// makes a directory in the filesystem
/////////////////////////////////////////////////////////////////////////
int my_mkdir(char *pathname){
    if(!pathname)
        return -1;
    char path_buff[128], dir_buff[128];
    strcpy(path_buff,pathname);
    strcpy(dir_buff,pathname);

    // split the pathname for the new dir into path and base
    char *parent = dirname(path_buff);
    char *dir = basename(dir_buff);
    printf("mkdir(): parent: %s\n",parent);
    printf("mkdir(): dir: %s\n",dir);

    int pino;
    // check if starting from root or cwd
    if(strcmp(parent,"/")==0){
        pino = 2;
    }
    else if(strcmp(parent,".")==0){
        pino = running->cwd->ino;
    }
    else{
        pino = getino(parent);
    }

    // check that the parent of the new dir is DIR type
    MINODE *pmip = iget(dev,pino);
    INODE *ip = &pmip->INODE;
    if(S_ISDIR(ip->i_mode)){
        // check if the parent dir already contains dir being made
        int isFound = search(pmip,dir);
        if(isFound == -1){
            my_mkdir_util(pmip,dir);
        }
        else{
            printf("DIR already exists!\n");
        }
    }
    pmip->INODE.i_links_count++;
    pmip->dirty=1;
    iput(pmip);
}