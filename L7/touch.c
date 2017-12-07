#include "l7.h"

/////////////////////////////////////////////////////////////////////////
// my_touch() change the last access time and last modification time
// return: none
/////////////////////////////////////////////////////////////////////////
int my_touch(char *pathname){
    int ino = getino(pathname);
    MINODE *mip = iget(dev,ino);
    INODE *ip = &mip->INODE;

    ip->i_atime = time(0L);
    mip->dirty = 1;
    iput(mip);
}