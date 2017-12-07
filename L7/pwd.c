#include "l7.h"

/////////////////////////////////////////////////////////////////////////
// rpwd() utility function
// return: none
/////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////
// pwd() prints the working directory
// return: none
/////////////////////////////////////////////////////////////////////////
int pwd(MINODE *wd){
    if(wd == root){
        printf("current working directory: /\n");
    }
    else{
        rpwd(wd);
    }
}