#include "l7.h"

int my_creat_util(MINODE *pmip, char *file){
    int ino = ialloc(dev);

    // create the new inode
    MINODE *mip = iget(dev,ino); // load the newly allocated inode into mem
    INODE *ip = &mip->INODE;
    ip->i_mode = (unsigned short)0x81A4; // REG type & permissions
    ip->i_uid = (unsigned short)running->uid; // owner uid
    ip->i_gid = (unsigned short)running->pid; // group id
    ip->i_size = 0;
    ip->i_links_count = (unsigned short)1;
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 0;
    for(int i = 0; i < 15;i++){
        ip->i_block[i] = 0;
    }
    mip->dirty = 1;
    iput(mip);

    // enter the newly created file as a file of the parent
    enter_child(pmip,ino,file);
}

int my_creat(char *pathname){
    char parCpy[128], fCpy[128];
    strcpy(parCpy,pathname);
    strcpy(fCpy,pathname);

    char *parent = dirname(parCpy);
    char *file = basename(fCpy);
    printf("my_creat(): parent: %s\n",parent);
    printf("my_creat(): file: %s\n",file);

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
        int isFound = search(pmip,file);
        if(isFound == -1){
            my_creat_util(pmip,file);
        }
    }
    pmip->dirty=1;
}