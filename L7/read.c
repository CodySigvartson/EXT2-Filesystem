#include "l7.h"
/*

**************** Algorithm of kread() in kernel ****************
int kread(int fd, char buf[ ], int nbytes, int space) //space=K|U
{
	(1). validate fd; ensure oft is opened for READ or RW;
	(2). if (oft.mode = READ_PIPE)
	return read_pipe(fd, buf, nbytes);
	(3). if (minode.INODE is a special file)
	return read_special(device,buf,nbytes);
	(4). (regular file):
	return read_file(fd, buf, nbytes, space);
}

*************** Algorithm of read regular files ****************
int read_file(int fd, char *buf, int nbytes, int space)
{
	(1). lock minode;
	(2). count = 0; // number of bytes read
	compute bytes available in file: avil = fileSize - offset;
	(3). while (nbytes)
	{
		compute logical block: lbk = offset / BLKSIZE;
		start byte in block: start = offset % BLKSIZE;
		(4). convert logical block number, lbk, to physical block number,
		blk, through INODE.i_block[ ] array;
		(5). read_block(dev, blk, kbuf); // read blk into kbuf[BLKSIZE];
		char *cp = kbuf + start;
		remain = BLKSIZE - start;
		(6) while (remain)// copy bytes from kbuf[ ] to buf[ ]
		{
			(space)? put_ubyte(*cp++, *buf++) : *buf++ = *cp++;
			offset++; count++; // inc offset, count;
			remain--; avil--; nbytes--; // dec remain, avil, nbytes;
		if (nbytes==0 || avail==0)
		{
			break;
		}
	}
(7). unlock minode;
(8). return count;
}

*/

/////////////////////////////////////////////////////////////////////////
// my_read() reads nbytes from an opened
//			 file descriptor into a buffer area in user space. 
//           read() invokes kread() in kernel,
//			 which implements the read system call
// return: -1 for read file
/////////////////////////////////////////////////////////////////////////

u32 map_lblk_blk(INODE *ip, int lblk);

char buf[BLKSIZE];

int my_read(int fd, char buf[], int nbytes){
    if(running->fd[fd]->mode == READ_MODE || running->fd[fd]->mode == RW_MODE){
        read_file(fd,buf,nbytes);

    }
    else{
        printf("Cannot read file. File was not opened for valid read mode\n");
        return -1;
    }
}

int read_file(int fd, char *buf, int nbytes){
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
    else if(12 <= lblk < 12+256){ // indirect block
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