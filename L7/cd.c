#include "l7.h"
/*

************ Algorithm of chdir *************
int chdir(char *pathname)
{
    (1). get INODE of pathname into a minode;
    (2). verify it is a DIR;
    (3). change running process CWD to minode of pathname;
    (4). iput(old CWD); return 0 for OK;
}

*/

/////////////////////////////////////////////////////////////////////////
// cd() change cwd to pathname
// return: ino of the file in the pathname
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
    MINODE *mip = iget(dev,ino); // MINODE inode in memory.
    if(S_ISDIR(mip->INODE.i_mode)){
        printf("cd(): inode is DIR...\n");
        iput(running->cwd);
        running->cwd = mip;
        printf("cd(): running->cwd changed\n");
        return ino;
    }
}