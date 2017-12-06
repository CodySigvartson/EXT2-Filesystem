#include "l7.h";

/**************** Algorithm of kwrite()  in kernel ****************
int kwrite(int fd, char *ubuf, int nbytes)
{
	(1). validate fd; ensure OFT is opened for write;
	(2). if(oft.mode = WRITE_PIPE)
			return write_pipe(fd, buf, nbytes);
	(3). if (minode.INODE is a special file)
			return write_special(device, buf.nbytes);
	(4). return write_file(fd, ubuf, nbytes);
}
*/

int write_file(fd, ubuf, nbytes)
{
	
}

int my_write(int fd, char *ubuf, int nbytes)
{
	//(1). validate fd; ensure OFT is opened for write;
	if(running->fd[fd]->mode == WRITE_MODE || running->fd[fd]->mode == RW_MODE || running->fd[fd]->mode == APPEND_MODE)
	{
		printf("%s mode opened!\n", running->fd[fd]->mode);
	}

	//(2). if(oft.mode = WRITE_PIPE)
	// OFT * oft = running->fd[fd];
	// if(oft.mode = WRITE_PIPE)
	// {
	// 		return write_pipe(fd, buf, nbytes);
	// }

	//(3). check special file
	// check if file has any special usages. i_mode bits 9-11 should be 000 if not special usage
	if(oft.mode & (1 << 9) != 0 && oft.mode & (1 << 10) != 0 && oft.mode & (1 << 11) != 0){
        printf("special file usage, unable to open\n");
        return 1;
    }
    return 0;

	//(4). write stuff into file
	return write_file(fd, ubuf, nbytes);
}