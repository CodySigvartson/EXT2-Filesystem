#include "l7.h"

/////////////////////////////////////////////////////////////////////////
// cd() change cwd to pathname
// return ino
/////////////////////////////////////////////////////////////////////////
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