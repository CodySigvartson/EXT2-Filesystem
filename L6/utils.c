/* Utilitiy functions for EXT2 Filesystem */
#include "l6.h"
#include <ext2fs/ext2_fs.h>
#include <stdio.h>
#include <fcntl.h>

MINODE minodes[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
MTABLE mtable[4]; 

SUPER *sp;
GD    *gp;
INODE *ip;

int dev;
int nblocks; // from superblock
int ninodes; // from superblock
int bmap;    // bmap block 
int imap;    // imap block 
int iblock;  // inodes begin block

// device
char *device;
// block data buff
char buff[BLKSIZE];
char buff2[BLKSIZE];
// dir names
char *names[64];

// enters a new dir into the parent directory
int enter_child(MINODE *pip,int ino, char *child){
    DIR *dp;
    char *cp;
    int ideal_len, remain, blk, i;
    int need_len = 4*((8+strlen(child)+3)/4);

    for(i = 0; i < 12; i++){
        blk = pip->INODE.i_block[i];
        if(blk == 0){
            printf("i %d\n",i);
            break;
        }
        get_block(dev,blk,buff);
        dp = (DIR *)buff;
        cp = buff;
        while(cp + dp->rec_len < buff + BLKSIZE){
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
        // dp points at last entry in parent's data block
        // get the amount of remaining space in the block
        ideal_len = 4*((8+dp->name_len+3)/4);
        remain = dp->rec_len - ideal_len;
        // add the new entry as the last entry
        if(remain >= need_len){
            dp->rec_len = ideal_len;

            cp += dp->rec_len;
            dp = (DIR *)cp;

            dp->inode = ino;
            dp->rec_len = (BLKSIZE - (cp - buff));
            dp->name_len = strlen(child);
            dp->file_type = (char)EXT2_FT_DIR;
            strcpy(dp->name,child);

            // write the block back to disk
            put_block(dev, blk, buff);
            return 1;
        }
    }

    // no space left in block, allocate new one
    blk = balloc(dev);
    pip->INODE.i_block[i] = blk;
    pip->INODE.i_size += BLKSIZE;
    pip->dirty = 1;

    get_block(dev,blk,buff);
    dp = (DIR *)buff;
    cp = buff;

    dp->inode = ino;
    dp->rec_len = BLKSIZE;
    dp->name_len = strlen(child);
    dp->file_type = (char)EXT2_FT_DIR;
    strcpy(dp->name,child);

    put_block(dev,blk,buff);
    return 1;
}


// tokenize a path into dir names
int tokenize(char *path){
    char *temp = strtok(path,"/");
    names[0] = temp;
    int i = 1;
    while(temp = strtok(0,"/")){
        names[i] = temp;
        i++;
    }
    names[i] = 0;
}

// get_block() reads a disk block into a buf[ ]
int get_block(int dev, int blk, char *buf)
{
    lseek(dev, (long)blk*BLKSIZE, SEEK_SET);
    return read(dev, buf, BLKSIZE);
}

// writes a block to disk
int put_block(int dev, int blk, char *buf){
    lseek(dev,(long)blk*BLKSIZE,SEEK_SET);
    write(dev,buf,BLKSIZE);
}

// searches for filename in given DIR
int search(MINODE *mip, char *name){
    printf("search(): searching for file: %s...\n",name);
    int blk;
    char *cp;
    char temp[256];
    INODE *tip = &mip->INODE;
    if(S_ISDIR(tip->i_mode)){
        for(int i = 0; i < 12; i++){
            if(tip->i_block[i]){
                blk = tip->i_block[i];
                get_block(dev,blk,buff);
                DIR *dp = (DIR *)buff;
                cp = buff;
                while(cp < buff + BLKSIZE){
                    strncpy(temp, dp->name,dp->name_len);
                    temp[dp->name_len] = 0;
                    if(strcmp(name,temp) == 0){
                        printf("file found at %d\n",dp->inode);
                        return dp->inode;
                    }
                    else{
                        cp += dp->rec_len;
                        dp = (DIR *)cp;
                    }
                }
            }
        }
    }
    return -1;
}

// checks if an minode already exists in memory
int minodeExists(int dev, int ino){
    //printf("minodeExists(): checking if inode exists in memory...\n");
    for(int i = 0; i < NMINODE; i++){
        if(minodes[i].ino == ino && minodes[i].dev == dev){
            return i;
        }
    }
    return -1;
}

// check for an unused minode
int minodeRef(){
    //printf("minodeRef(): looking for unused location in memory...\n");
    for(int i = 0; i < NMINODE; i++){
        if(minodes[i].refCount == 0){
            return i;
        }
    }
    return -1;
}

// gets an INODE from the device
MINODE *iget(int dev, int ino){
    //printf("iget(): getting inode from device...\n");
    // search if the minode is already in use
    int loc = minodeExists(dev,ino);
    if(loc >= 0){
        minodes[loc].refCount++;
        //printf("iget(): inode is already in memory, now referenced %d times\n");
        return &minodes[loc];
    }
    //printf("iget(): inode does not already exist in mem\n");
    // search for an unused minode
    int unused = minodeRef();
    //printf("iget(): found unused location at %d\n",unused);
    if(unused >= 0){
        minodes[unused].refCount = 1;
        minodes[unused].dev = dev;
        minodes[unused].ino = ino;
        minodes[unused].dirty = 0;
        minodes[unused].mounted = 0;
        minodes[unused].mptr = NULL;
    }
    else{
        //printf("iget(): Unable to find an unused inode.\n");
        return NULL;
    }


    // get location of inode on device
    int blk = (ino - 1)/8 + 10;
    int offset = (ino - 1) % 8;
    //printf("iget(): inode location %d on block %d\n",offset,blk);

    get_block(dev,blk,buff);
    INODE *ip = (INODE *)buff;
    ip += offset;
    minodes[unused].INODE = *ip;

    return &minodes[unused];
}

// writes an minode back to its disk
int iput(MINODE *mip){
    printf("writing inode back to disk...\n");
    int block, offset;

    // remove a reference from this minode
    mip->refCount--;
    printf("ref count is now %d\n",mip->refCount);

    // check if minode is being used elsewhere
    if(mip->refCount > 0){
        return;
    }
    // check if mip was modified
    if(!mip->dirty){
       return;
    }

    // get the minode location on disk
    block = mip->ino / 8 + 10;
    offset = mip->ino % 8;

    get_block(dev,block,buff);

    ip = (INODE *)buff + offset;
    *ip = mip->INODE;

    // write the minode back to its disk
    put_block(mip->dev,block,buff);
    printf("write back to disk succeeded\n");
}

int getino(char *pathname)
{
    int i, ino, blk, disp;
    char buf[BLKSIZE];
    INODE *ip;
    MINODE *mip;
    dev = root->dev; // only ONE device so far

    printf("getino(): pathname=%s\n", pathname);
    if (strcmp(pathname, "/")==0)
        return 2;

    if (pathname[0]=='/'){
        mip = root;
    }
    else{
        printf("getino(): entering iget()\n");
        mip = iget(dev, running->cwd->ino);
    }

    strcpy(buf, pathname);
    tokenize(buf); // n = number of token strings

    // get the number of dirs
    int len = 0;
    while(names[len]){
        len++;
    }
    int n = len;
    printf("getino(): number of dirs to check = %d\n",n);

    // check each dir in the path name
    for (i=0; i < n; i++){
        printf("===========================================\n");
        printf("getino(): i=%d name[%d]=%s\n", i, i, names[i]);
        
        // search for the dir
        ino = search(mip, names[i]);

        // check if dir exists
        if (ino==0){
            iput(mip);
            printf("getino(): name %s does not exist\n", names[i]);
            return 0;
        }
        // dir exists
        iput(mip);
        mip = iget(dev, ino);
    }
    iput(mip);

    return ino;
}

// allocates an inode on device
int ialloc(int dev){
    char buf[BLKSIZE];
    // get imap block
    get_block(dev,imap,buf);
    for(int i = 0; i < ninodes; i++){
        if(test_bit(buf,i)==0){
            set_bit(buf,i);
            put_block(dev,imap,buf);
            dec(dev,0);
            printf("inode alloc at: %d\n",i+1);
            return (i+1); // +1 to go back to 1 index for reuse in iget
        }
    }
    printf("no inodes!!!\n");
    return 0; // no free inodes available
}

// allocates a free disk block on device
int balloc(int dev){
    char buf[BLKSIZE];
    // get block bitmap
    get_block(dev,bmap,buf);
    for(int i = 10; i < nblocks;i++){
        if(test_bit(buf,i)==0){
            set_bit(buf,i);
            put_block(dev,bmap,buf);
            dec(dev,1);
            return (i+1); // +1 to go back to 1 index for reuse in iget
        }
    }
    return 0; // no block available
}

// mounts the root of the filesystem from the device
int mount_root(){
    printf("ATTEMPTING TO MOUNT FILESYSTEM...\n");
    // open the disk
    printf("opening device...\n");
    dev = open(device,O_RDWR);
    if(dev < 0){
        printf("Unable to open device: %s\n",device);
        exit(1);
    }
    printf("device: %s opened\n",device);
    // get superblock to check for EXT2 FS
    get_block(dev,SBLK,buff);
    sp = (SUPER *)buff;
    if(sp->s_magic != 0xEF53){
        printf("Invalid filesystem. Filesystem should be of type EXT2.\n");
        exit(2);
    }
    else{
        printf("EXT2 filesystem check: OK\n");
    }

    nblocks = sp->s_blocks_count;
    printf("nblocks: %d\n",nblocks);
    ninodes = sp->s_inodes_count;
    printf("ninodes: %d\n",ninodes);
    
    get_block(dev,GDBLK,buff);
    gp = (GD *)buff;

    bmap = gp->bg_block_bitmap;
    printf("bmap: %d\n",bmap);
    imap = gp->bg_inode_bitmap;
    printf("imap: %d\n",imap);
    iblock = gp->bg_inode_table;
    printf("iblock: %d\n",iblock);

    // set root
    root = iget(dev,2);
    printf("root set\n");

    mtable[0].dev = dev;
    mtable[0].nblock = nblocks;
    mtable[0].ninodes = ninodes;
    mtable[0].bmap = bmap;
    mtable[0].imap = imap;
    mtable[0].iblock = iblock;
    mtable[0].mountDirPtr = root;
    strcpy(mtable[0].deviceName,device);
    strcpy(mtable[0].mountedDirName,"/");

    proc[0].cwd = iget(dev,2);
    printf("proc[0] set to root\n");
    proc[1].cwd = iget(dev,2);
    printf("proc[1] set to root\n");

    running = &proc[0];
    printf("running set to proc[0] cwd\n");
    printf("EXT2 FILESYSTEM MOUNTED SUCCESFULLY\n\n");
}

// initializes FS data structures
int init(){
    printf("Initializing FS data structures...\n");
    // initialize all minodes
    for(int i = 0; i < NMINODE;i++){
        minodes[i].dev = 0;
        minodes[i].ino = 0;
        minodes[i].refCount = 0;
        minodes[i].dirty = 0;
        minodes[i].mounted = 0;
        minodes[i].mptr = NULL;
    }

    // initialize root
    root = NULL;

    // initialize processes
    for(int i = 0; i < NPROC; i++){
        proc[i].next = NULL;
        proc[i].pid = 0;
        proc[i].uid = 0;
        proc[i].cwd = NULL;
        for(int j = 0; j < NFD; j++){
            proc[i].fd[j] = NULL;
        }
    }
    proc[1].uid = 1;
    running = NULL;

    // initialize mount table
    for(int i = 0; i < 4; i++){
        mtable[i].dev = 0;
        mtable[i].nblock = 0;
        mtable[i].ninodes = 0;
        mtable[i].bmap = 0;
        mtable[i].imap = 0;
        mtable[i].iblock = 0;
        mtable[i].mountDirPtr = NULL;
    }

    // initialize FS structs
    sp = NULL;
    gp = NULL;
    ip = NULL;
    printf("FS data structure initialization complete...\n");
    printf("--------------------------------------------\n");
}