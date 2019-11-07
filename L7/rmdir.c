#include "l7.h"

/*

************ Algorithm of Delete_dir_entry (name) *************
(1). search DIR's data block(s) for entry by name;

(2). if (entry is the only entry in block)
        clear entry’s inode number to 0;
    else
    {
        (3). if (entry is last entry in block)
        add entry's rec_len to predecessor entry's rec_len;
        (4). else{ // entry in middle of block
        add entry's rec_len to last entry's rec_len;
        move all trailing entries left to overlay deleted entry;
    }
}
(5). write block back to disk;


************ Algorithm of rmdir *************
rmdir(char *pathname)
{
    1. get in- memory INODE of pathname:
    ino = getino(&de, pathanme);
    mip = iget(dev,ino);
    2. verify INODE is a DIR (by INODE.i_mode field);
    minode is not BUSY (refCount = 1);
    DIR is empty (traverse data blocks for number of entries = 2);
    3. get parent's ino and inode 
    pino = findino(); //get pino from .. entry in INODE.i_block[0]
    pmip = iget(mip->dev, pino);
    4. remove name from parent directory
    findname(pmip, ino, name); //find name from parent DIR
    rm_child(pmip, name);
    5. deallocate its data blocks and inode
    truncat(mip); // deallocate INODE's data blocks
    6. deallocate INODE
    idalloc(mip->dev, mip->ino); iput(mip);
    7. dec parent links_count by 1;
    mark parent dirty; iput(pmip);
    8. return 0 for SUCCESS.
}
*/

/////////////////////////////////////////////////////////////////////////
// my_rmdir() removing a non-empty directory implies removing all the files
//            and subdirectories in the directory
// return: -1 for error
/////////////////////////////////////////////////////////////////////////
int my_rmdir(char *pathname){
    if(!pathname){
        printf("Please provide a pathname for the directory to remove\n");
        return -1;
    }
    int count, pino;
    // get the in-memory inode of the pathname
    int ino = getino(pathname);
    printf("my_rmdir(): dir to remove found at ino %d\n",ino);
    MINODE *mip = iget(dev,ino);
    INODE *ip = &mip->INODE;
    // verify the directory to remove is empty
    if(S_ISDIR(ip->i_mode) && mip->refCount == 1){
        for(int i = 0; i < 12; i++){
            count = 0;
            if(ip->i_block[i]){
                if(ip->i_links_count > 2){
                    printf("unable to remove dir: not empty\n");
                    return -1;
                }
                int blk = ip->i_block[i];
                get_block(dev,blk,buff);
                char *cp;
                DIR *dp = (DIR *)buff;
                cp = buff2;
                while(cp < buff + BLKSIZE){
                    count++;
                    cp += dp->rec_len;
                    dp = (DIR *)cp;
                }
                if(count > 2){
                    printf("unable to remove dir: not empty\n");
                    return -1;
                }
            }
        }
    }
    
    printf("my_rmdir(): dir is empty, OK to remove\n");
    char parPath[128], dirName[128];
    strcpy(parPath,pathname);
    strcpy(dirName, pathname);
    char *par = dirname(parPath);
    printf("my_rmdir(): parent: %s\n",par);
    char *dir = basename(dirName);
    printf("my_dir(): dir: %s\n",dir);
    if(strcmp(par,"/")==0){
        pino = 2;
    }
    else if(strcmp(par,".")==0){
        pino = running->cwd->ino;
    }
    else{
        pino = getino(par);
    }
    // remove the dir from parent's data block
    MINODE *pmip = iget(dev,pino);

    rm_child(pmip,ino,dir);
    bdalloc(dev,ip->i_block[0]);
    idalloc(dev,mip->ino);
    iput(mip);

    pmip->INODE.i_links_count--;
    pmip->dirty = 1;
    iput(pmip);
}

/////////////////////////////////////////////////////////////////////////
// rm_child() remove a child from a parent node
// return: none
/////////////////////////////////////////////////////////////////////////
int rm_child(MINODE *pmip, int ino, char *name){
    int i,blk;
    int found = 0;
    DIR *dp,*dpend,*prev;
    char *cp, *cpend;
    INODE *ip = &pmip->INODE;

    // find the dir to remove in the DIRECT blocks
    for(i = 0; i < 12; i++){
        blk = ip->i_block[i];
        if(blk){
            printf("rm_child(): dir to remove in blk %d\n",blk);
            get_block(dev,blk,buff);
            dp = (DIR *)buff;
            cp = buff;
            // traverse through to find the dir to remove
            while(cp < buff + BLKSIZE){
                char dir[128];
                strcpy(dir,dp->name);
                dir[dp->name_len]=0;
                if(strcmp(name, dir)==0 && ino == dp->inode){
                    printf("rm_child(): found dir to remove in the data block\n");
                    found = 1;
                    break;
                }
                cp += dp->rec_len;
                prev = dp;
                dp = (DIR *)cp;
            }
            if(found)
                break;
        }
    }
    if(!found){
        printf("rm_child(): dir not found to remove\n");
        return -1;
    }

    // dir has been found to remove, dp points to it
    // case dir is first item and only item in the data block:
    if(cp == buff && cp + dp->rec_len == buff + BLKSIZE){
        printf("rm_child(): removing dir as first and only item in block\n");
        bdalloc(dev,ip->i_block[i]);
        ip->i_size -= BLKSIZE;
        // compact the parent's i_block[] 
        while(ip->i_block[i+1] && i+1 < 12){
            i++;

            get_block(dev,ip->i_block[i],buff);
            put_block(dev,ip->i_block[i-1],buff);
        }
    }
    // if entry is the very last entry
    else if(cp + dp->rec_len == buff + BLKSIZE){
        printf("rm_child(): dir to remove is at end\n");
        // take add one file size to previous dir
        prev->rec_len += dp->rec_len;
        put_block(dev,ip->i_block[i],buff);
    }
    // entry is in the middle
    else{
        printf("rm_child(): found dir to remove in middle\n");
        // get the last dir
        get_block(dev,blk,buff);
        dpend = (DIR *)buff;
        cpend = buff;
        while(cpend + dpend->rec_len < buff + BLKSIZE){
            cpend += dpend->rec_len;
            dpend = (DIR *)cpend;
        }
        // adding the removed dir size to the last dir size
        dpend->rec_len += dp->rec_len;

        // move proceeding entries left
        unsigned char *start = cp+dp->rec_len;
        unsigned char *end = buff + BLKSIZE;
        memmove(cp,start,end-start);

        put_block(dev,ip->i_block[i],buff);
    }

    pmip->dirty=1;
    iput(pmip);
}