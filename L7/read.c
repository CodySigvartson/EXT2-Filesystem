#include "l7.h"

u32 map_lblk_blk(INODE *ip, int lblk);

char buf[BLKSIZE];

int my_read(int fd, char buf[], int nbytes, int space){
    if(running->fd[fd]->mode == READ_MODE || running->fd[fd]->mode == RW_MODE){
        read_file(fd,buf,nbytes,space);
    }
    else{
        printf("Cannot read file. File was not opened for valid read mode\n");
        return -1;
    }
}

int read_file(int fd, char *buf, int nbytes, int space){
    OFT *oftp = running->fd[fd];
    // lock minode
    oftp->mptr->lock = 1;
    MINODE *mip = oftp->mptr;
    INODE *ip = &mip->INODE;
    int count = 0; // number of bytes read
    // get number of bytes to read
    int bytesAvail = (ip->i_size - oftp->offset);

    while(nbytes){
        // get logical block
        u32 lblk = oftp->offset / BLKSIZE;
        // get the start byte in the block
        int startByte = oftp->offset % BLKSIZE;
        // get physical block location
        u32 blk = map_lblk_blk(ip,lblk);
        // read the block into buff
        get_block(dev,blk,buff);
        // point to start reading location
        char *cp = buff + startByte;
        int remain = BLKSIZE - startByte;

        // get the size of bytes to read
        int sizeToCopy;
        if(nbytes < remain) // check if getting all bytes requested
            sizeToCopy = nbytes;
        else // bytes requested is greater than what is remaining to copy
            sizeToCopy = remain;
        
        // copy data in to buffer
        memcpy(buf,buff,sizeToCopy);
        // update vars
        count += sizeToCopy;
        nbytes -= sizeToCopy;
        bytesAvail -= sizeToCopy;
        oftp->offset += sizeToCopy;
    }
    return count;
}

// convert the logical block number to physical block in mem
u32 map_lblk_blk(INODE *ip, int lblk){
    u32 blk;
    if(lblk < 12){ // return direct blk
        blk = ip->i_block[lblk];
    }
    else if(12 <- lblk < 12+256){ // indirect block
        u32 ibuf[256];
        memcpy(ibuf,ip->i_block[12],sizeof(ip->i_block[12]));
        blk = ibuf[lblk-12];
    }
    else{ // double indirect blocks
        u32 dbuf[256];
        memcpy(dbuf,ip->i_block[13],sizeof(ip->i_block[13]));
        lblk -= (12+256);
        u32 dblk = dbuf[lblk/256];
        memset(dbuf,0,sizeof(dbuf));
        memcpy(dbuf,dblk,sizeof(dblk));
        blk = dbuf[lblk % 256];
    }
    return blk;
}