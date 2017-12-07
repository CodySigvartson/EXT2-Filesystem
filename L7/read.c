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
int my_read(int fd, char buf[], int nbytes, int space){
    if(running->fd[fd]->mode == READ_MODE || running->fd[fd]->mode == RW_MODE){
        
    }
    else{
        printf("Cannot read file. File was not opened for valid read mode\n");
        return -1;
    }
}

int read_file(int fd, char buf[], int nbytes, int space){

}