#include "l7.h"

int ls(char *dirname){
    int ino;
    if(!dirname){
        // ls cwd
        ino = running->cwd->ino;
    }
    else{
        printf("ls_dir(): calling...\n");
        ino = getino(dirname);
        printf("ls_dir(): ino of dir is %d\n",ino);
    }
    // get the inode to ls in memory
    MINODE *mip = iget(dev,ino);
    INODE *tip = &mip->INODE;
    int blk = tip->i_block[0];
    get_block(dev,blk,buff2);
    char *cp;
    DIR *dp = (DIR *)buff2;
    cp = buff2;
    while(cp < buff2 + BLKSIZE){
        int ino = dp->inode;
        ls_file(ino);
        char name[256];
        strcpy(name,dp->name);
        name[dp->name_len] = 0;
        printf("%12s\n",name);
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
}

int ls_file(int ino){
    MINODE *mip = iget(dev,ino);
    INODE *ip = &mip->INODE;
    if(!mip){
        printf("ls_file(): file does not exist...\n");
        return;
    }
    // get file info (mode,permissions, uid,gid, size, last modified, name)
    // permissions
    if(mip->INODE.i_mode & 1 << 8)
        putchar('r');
    else putchar ('-');
    if(mip->INODE.i_mode & 1 << 7)
        putchar('w');
    else putchar ('-');
    if(mip->INODE.i_mode & 1 << 6)
        putchar('x');
    else putchar ('-');

    if(mip->INODE.i_mode & 1 << 5)
        putchar('r');
    else putchar ('-');
    if(mip->INODE.i_mode & 1 << 4)
        putchar('w');
    else putchar ('-');
    if(mip->INODE.i_mode & 1 << 3)
        putchar('x');
    else putchar ('-');

    if(mip->INODE.i_mode & 1 << 2)
        putchar('r');
    else putchar ('-');
    if(mip->INODE.i_mode & 1 << 1)
        putchar('w');
    else putchar ('-');
    if(mip->INODE.i_mode & 1 << 0)
        putchar('x');
    else putchar ('-');

    // file type
    if((ip->i_mode & (unsigned short)0xF000)==(unsigned short)0x8000){
        printf("%3s","r");
    }
    else if((ip->i_mode & (unsigned short)0xF000)==(unsigned short)0x4000){
        printf("%3s","d");
    }
    else{
        printf("%3s","l");
    }

    // uid and gid
    printf("%3hu%3hu%3hu%8d",ip->i_links_count,ip->i_uid,ip->i_gid,ip->i_size);

}