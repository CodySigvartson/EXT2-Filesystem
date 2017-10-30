#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#include "l4.c"

int main(int argc, char *argv[]){
    int ino;
    if(argc < 3){
        printf("please provide disk and pathname: ./a.out myDisk myPath\n");
        exit(2);
    }

    printf("Opening disk: %s...\n",argv[1]);
    int dev = open(argv[1],O_RDWR);
    if(dev > 0){
        printf("Device opened!\n");
    }

    if(!(strcmp(argv[2],"/")==0))
        ino = getino(argv[2]);
    else ino = 2;

    MINODE *mip = iget(dev,ino);

    INODE *ip = &mip->INODE;
    printf("getting i_blocks for path: %s\n",argv[2]);
    for(int i = 0; i < 15; i++){
        if(i <= 11)
            printf("indirect i_block[%d] = %d\n",i,ip->i_block[i]);
        if(i == 12)
            printf("direct i_block[%d] = %d\n",i,ip->i_block[i]);
        if(i == 13)
            printf("double-direct i_block[%d] = %d\n",i,ip->i_block[i]);
        if(i == 14)
            printf("triple-direct i_block[%d] = %d\n",i,ip->i_block[i]);
    }
    

    return 0;
}