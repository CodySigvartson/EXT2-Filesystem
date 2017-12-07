#include "l7.h";

/*

*************** Algorithm of kwrite() in kernel ****************
int kwrite(int fd, char *ubuf, int nbytes)
{
	(1). validate fd; ensure OFT is opened for write;
	(2). if(oft.mode = WRITE_PIPE)
			return write_pipe(fd, buf, nbytes);
	(3). if (minode.INODE is a special file)
			return write_special(device, buf.nbytes);
	(4). return write_file(fd, ubuf, nbytes);
}

*************** Algorithm of write() in kernel ****************
int write_file(int fd, char *buf, int nbytes)
{
	(1). lock minode;
	(2). count = 0;		// number of bytes written
	(3). while(nbytes)
	{
		compute logical block: lbk = oftp->offset / BLOCK_SIZE;
		compute start byte: start = oftp->offset % BLOCK_SIZE;
	}
	(4). convert lbk to physical block number, blk;
	(5). read_block(dev, blk, kbuf); // read blk into kbuf[BLKSIZE];
		char *cp = kbuf +start; remain = BLKSIZE - start;
	(6). while(remain)
	{
		put_ubyte(*cp++, *ubuf++);
		offset++;	// inc offset
		count++;	// inc count;
		remain--;	// dec remain;
		nbytes--;	// dec nbytes;
		if(offset > fileSize)
		{
			fileSize++; // inc file size
		}
		if(nbytes <= 0)
		{
			break;
		}
		(7). write_block(dev, blk, kbuf);
	}
	set minode dirty = 1;	// mark minode dirty for iput()
	unlock(minode);
	return count;
}
*/

/////////////////////////////////////////////////////////////////////////
// write_file() writes nbytes from ubuf in user space to an opened file 
// 				descriptor and returns the actual number of bytes written
// return: none
// The algorithm of converting logical block to physical block for write is similar
// to that of read, except for the following difference. During write, the intended
// data block may not exist. If a direct block does not exist, it must be allocated and
// recorded in the INODE. If the indirect block does not exist, it must be allocated
// and initialized to 0. If an indirect data block does not exist, it must be allocated
// and recorded in the indirect block, etc. The reader may consult the write.c file for
// details.
/////////////////////////////////////////////////////////////////////////
int write_file(fd, ubuf, nbytes)
{
	printf("inside write_file()\n");
	/**************** Algorithm of write() regular file ****************
	int write_file(int fd, char *ubuf, int nbytes)
	{
		(1). lock minode;
		(2). count = 0; // number of bytes written
				compute logical block: lbk = oftp->offset / BLOCK_SIZE;
				compute start byte: start = oftp->offset % BLOCK_SIZE;
		(3). while(nbytes)
			{
				compute logical block: lbk = oftp->offset / BLOCK_SIZE;
				compute start byte: start = oftp->offset % BLOCK_SIZE;
				
			}
	}
	*/






}

int my_write(int fd, char *ubuf, int nbytes)
{
	OFT * oft = running->fd[fd];

	//(1). validate fd; ensure OFT is opened for write;
	if(running->fd[fd]->mode == WRITE_MODE || running->fd[fd]->mode == RW_MODE || running->fd[fd]->mode == APPEND_MODE)
	{
		printf("%s mode opened!\n", running->fd[fd]->mode);
	}
	//(2). if(oft.mode = WRITE_PIPE)
	// if(oft.mode = WRITE_PIPE)
	// {
	// 		return write_pipe(fd, buf, nbytes);
	// }

	//(3). check special file
	// check if file has any special usages. i_mode bits 9-11 should be 000 if not special usage
	if(oft->mode & (1 << 9) != 0 && oft->mode & (1 << 10) != 0 && oft->mode & (1 << 11) != 0){
        printf("special file usage, unable to open\n");
        return 1;
    }

	//(4). write stuff into file
	return write_file(fd, ubuf, nbytes);
}