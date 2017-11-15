#include "l7.h"

// reads a target file name symbolic link field
int my_readlink(char *file, char *buffer){
    int ino = getino(file);
    MINODE *mip = iget(dev,ino);
    INODE *ip = &mip->INODE;
    if(S_ISLNK(ip->i_mode)){
        memcpy(buffer,ip->i_block,strlen(ip->i_block));
    }
    iput(mip);
    return strlen(buffer);
}