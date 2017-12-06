#include "l7.h"

int check_special_file(u16 mode);
int check_file_open(MINODE *mip);
int wipe_contents(INODE *ip);

// flags:
// 0: R
// 1: W
// 2: RW
// 3: APPEND
int my_open(char *pathname, int flag){
    // get the inode into memory
    int ino = getino(pathname);
    if(ino == 0){
        printf("file does not exist, creating file to open\n");
        my_creat(pathname);
        ino = getino(pathname);
    }
    MINODE *mip = iget(dev,ino);
    INODE *ip = &mip->INODE;
    // check opening regular file
    if(S_ISREG(ip->i_mode)){
        // check access permissions for non-special file
        if(!check_special_file(ip->i_mode)){
            printf("no special file usage\n");
            // check if file is open
            if(!check_file_open(mip)){
                printf("file not open, OK to continue\n");
                OFT *oftp = malloc(sizeof(OFT));
                int byteOffset;
                switch(flag){
                    case 0: //r
                        byteOffset = 0;
                        break;
                    case 1: //w
                        wipe_contents(ip);
                        byteOffset = 0;
                        break;
                    case 2: //rw
                        byteOffset = 0;
                        break;
                    case 3: //append (w with offset=file_size)
                        byteOffset = ip->i_size;
                        break;
                    default:
                        printf("invalid file mode\n");
                        return -1;
                }
                // initialize open file table
                oftp->mode = flag;
                oftp->mptr = mip;
                oftp->offset = byteOffset;
                oftp->refCount++;
                // check for lowest fd index
                int fd;
                for(int i = 0; i < NFD; i++){
                    if(running->fd[i] == 0){
                        fd = i;
                        break;
                    }
                    if(i == NFD - 1){
                        printf("no file descriptors available for this PROC\n");
                        return -1;
                    }
                }

                running->fd[fd] = oftp;
                
                if(flag == 0)
                    ip->i_atime = time(0L);
                else
                    ip->i_atime = ip->i_mtime = time(0L);
                mip->lock = 0; // unlock
                return fd;
            }
        }
    }
}

// check if file has any special usages. i_mode bits 9-11 should be 000 if not special usage
int check_special_file(u16 mode){
    if(mode & (1 << 9) != 0 && mode & (1 << 10) != 0 && mode & (1 << 11) != 0){
        printf("special file usage, unable to open\n");
        return 1;
    }
    return 0;
}

// checks if file is already open
int check_file_open(MINODE *mip){
    // go through all file descriptors, check if it's for the file, check if open in a write mode
    for(int i = 0; i < NFD; i++){
        if(running->fd[i] != 0 && running->fd[i]->mptr != mip && running->fd[i]->mode > 0){
            printf("file is already opened for a write mode, unable to open\n");
            return 1;
        }
    }
    return 0;
}

// wipes file contents for user opening file in write mode
int wipe_contents(INODE *ip){
    for(int i = 0; i < 14; i++){
        ip->i_block[i] = 0;
    }
}

// level 3 permissions
// int check_permissions(char *pathname, int flag){
//     int ino = getino(pathname);
//     MINODE *mip = iget(dev,ino);
//     INODE *ip = &mip->INODE;

//     switch(flag){
//         case 0: // r
//             if(ip->i_mode & )
//             {
                
//             }
//             break;
//         case 1: // w
//             if(ip->i_mode & 0222){

//             }
//             break;
//         case 2: // rw
//             if(ip->i_mode & 0666){

//             }
//             break;
//         case 3: // w - append
//             if(ip->i_mode & 0222){

//             }
//             break;
//     }
// }