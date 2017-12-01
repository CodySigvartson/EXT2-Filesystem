#include "l7.h"

int my_chmod(char *mode, char *pathname){
    int ino = getino(pathname);
    MINODE *mip = iget(dev,ino);
    INODE *ip = &mip->INODE;

    // clear current permissions
    ip->i_mode = ip->i_mode & 0xF000;

    int permissions = atoi(mode);
    switch(permissions){
        case 400: // owner: r
            ip->i_mode = ip->i_mode | 0400;
            break;
        case 040: // group: r
            ip->i_mode = ip->i_mode | 0040;
            break;
        case 004: // anyone: r
            ip->i_mode = ip->i_mode | 0004;
            break;
        case 200: // owner: w
            ip->i_mode = ip->i_mode | 0200;
            break;
        case 020: // group: w
            ip->i_mode = ip->i_mode | 0020;
            break;
        case 002: // anyone: w
            ip->i_mode = ip->i_mode | 0002;
            break;
        case 100: // owner: x
            ip->i_mode = ip->i_mode | 0100;
            break;
        case 010: // group: x
            ip->i_mode = ip->i_mode | 0010;
            break;
        case 001: // anyone: x
            ip->i_mode = ip->i_mode | 0001;
            break;
        case 444: // all: r
            ip->i_mode = ip->i_mode | 0444;
            break;
        case 777: // all: rwx
            ip->i_mode = ip->i_mode | 0777;
            break;
    }
    mip->dirty = 1;
    iput(mip);
}