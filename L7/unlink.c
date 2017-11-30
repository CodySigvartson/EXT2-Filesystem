#include "l7.h"

/////////////////////////////////////////////////////////////////////////
// my_unlink() unlinks a file
/////////////////////////////////////////////////////////////////////////
int my_unlink(char *filename){
    char path_buff[128], file_buff[128];
    strcpy(path_buff,filename);
    strcpy(file_buff,filename);

    int ino = getino(filename);
    MINODE *mip = iget(dev,ino);
    if(S_ISREG(mip->INODE.i_mode) || S_ISLNK(mip->INODE.i_mode)){
        char *path = dirname(path_buff);
        char *file = basename(file_buff);

        int parentino = getino(path);
        MINODE *parentmip = iget(dev,parentino);
        rm_child(parentmip,ino,file);
        parentmip->dirty=1;
        iput(parentmip);

        mip->INODE.i_links_count--;
        if(mip->INODE.i_links_count > 0)
            mip->dirty = 1;
        else{ // i_links_count == 0, dealloc
            for(int i = 0; i < 15; i++){
                int blk = mip->INODE.i_block[i];
                bdalloc(dev,blk);
            }
            idalloc(dev,mip->ino);
        }
        iput(mip);
    }
}