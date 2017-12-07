#include "l7.h"

/*


*********** Algorithm of unlink *************
unlink(char *filename)
{
    1. get filenmae's minode:
    ino = getino(&dev, filename);
    mip = iget(dev, ino);
    check it's a REG or SLINK file
    2. // remove basename from parent DIR
    rm_child(pmip, mip->ino, basename);
    pmip->dirty = 1;
    iput(pmip);
    3. // decrement INODE's link_count
    mip->INODE.i_links_count--;
    if (mip->INODE.i_links_count > 0)
    {
        mip->dirty = 1; iput(mip);
    }
    4. if (!SLINK file) // assume:SLINK file has no data block
    truncate(mip); // deallocate all data blocks
    deallocate INODE;
    iput(mip);
}


*/


/////////////////////////////////////////////////////////////////////////
// my_unlink() decrements the file’s links_count by 1 and deletes the file name from its
//             parent DIR. When a file’s links_count reaches 0, 
//             the file is truly removed by deallocating its data blocks and inode.
// return: none
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
    else{
        printf("Unable to unlink DIR type!\n");
    }
}