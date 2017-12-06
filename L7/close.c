#include "l7.h"

/************* Algorithm of close() *****************
int close(int fd)
{
	(1). check fd is a valid opened file descriptor;
	(2). if (PROC's fd[fd] != 0)
	{
		(3). if (openTable's mode == READ/WRITE PIPE)
		{
			return close_pipe(fd); // close pipe descriptor
		}

		(4). if (--refCount == 0) // if last process using this OFT
		{
			lock(minodeptr);
			iput(minode); // release minode
		}
	}
(5). clear fd[fd] = 0; // clear fd[fd] to 0
(6). return SUCCESS;
}
*/


/////////////////////////////////////////////////////////////////////////
// my_close() close file after file has been modified or created
// return: -1 for error, 0 for no file, 1 for there is a file
/////////////////////////////////////////////////////////////////////////
int my_close(int fd)
{
	printf("1. verify fd is within range.\n");
	if (fd < 0 || fd > NFD) // number file descriptor
	{
		printf("fd is out of range.\n");
		return -1;
	}

	oft = running->fd[fd];

	if(oft != 0)
	{
		// if (openTable mode == READ/WRITE PIPE)
		// {
		// 	return close_pipe(fd); // close pipe descriptor
		// }

		if (--oft->refCount == 0) // if last process using this OFT
		{
			MINODE *mip = oft->mptr; //last user of this OFT entry ==> dispose of the Minode[]

			oft->lock = 1; // 1 for lock, 0 for unlock

			iput(mip); // release minode and write stuff back to mydisk
		}

		running->fd[fd] = 0; // clear fd[fd] to 0
	}

	return 0; // return success
}