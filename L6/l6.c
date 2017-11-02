#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

//#include "utils.c"
#include "l6.h"
#include "utils.c"
#include "cd.c"
#include "mkdir.c"
#include "pwd.c"
#include "ls.c"
#include "creat.c"

MINODE minodes[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
MTABLE mtable[4]; 

SUPER *sp;
GD    *gp;
INODE *ip;

int dev;
int nblocks; // from superblock
int ninodes; // from superblock
int bmap;    // bmap block 
int imap;    // imap block 
int iblock;  // inodes begin block

// device
char *device;
// block data buff
char buff[BLKSIZE];
char buff2[BLKSIZE];
// dir names
char *names[64];

#define CMD_BUFF 128

// possible commands
char *cmds[] = {"cd","pwd","ls","mkdir","rmdir","creat","quit"};
// user input (command, command buffer)
char cmd[CMD_BUFF], cmdbuff[CMD_BUFF];
char *myargv[64];

// tokenizes user input into command and params
int tokenizeCmd(char *cmd){
    char *temp = strtok(cmdbuff," ");
    myargv[0] = temp;
    int i = 1;
    while(temp = strtok(0," ")){
        myargv[i] = temp;
        i++;
    }
    myargv[i] = 0;
}

// finds command entered by user
int findCmd(char *cmd){
    if(cmd){
        int i = 0;
        while(cmds[i]){
            if(strcmp(cmd,cmds[i])==0){
                return i;
            }
            i++;
        }
    }
    return -1;
}

// test bit value
int test_bit(char *buf, int bit){
    return buf[bit/8] & (1 << (bit % 8));
}

// set bit value
int set_bit(char *buf, int bit){
    buf[bit/8] |= (1 << (bit % 8));
}

// clear any bit to 0
int clear_bit(char *buf, int bit){
    buf[bit/8] &= ~(1 << (bit%8));
}

// increment number of free inodes or blocks
// blktype: 0 - inode, 1 - block
int inc(int dev, int blktype){
    get_block(dev,SBLK,buff2);
    sp = (SUPER *)buff2;
    if(blktype == 0)
        sp->s_free_inodes_count++;
    else if(blktype == 1)
        sp->s_free_blocks_count++;
    put_block(dev,SBLK,buff2);
    get_block(dev,GDBLK,buff2);
    gp = (GD *)buff2;
    if(blktype == 0)
        gp->bg_free_inodes_count++;
    else if(blktype == 1)
        gp->bg_free_blocks_count++;
    put_block(dev,GDBLK,buff2);
}

int idalloc(int dev,int ino){
    int i;
    if(ino > ninodes){
        printf("inumber %d out of range \n",ino);
        return;
    }
    get_block(dev,imap,buff2);
    clear_bit(buff2,ino-1);
    put_block(dev,imap,buff2);
    inc(dev,0);
}

int bdalloc(int dev,int bno){
    int i;
    if(bno > nblocks){
        printf("inumber %d out of range \n",bno);
        return;
    }
    get_block(dev,bmap,buff2);
    clear_bit(buff2,bno-1);
    put_block(dev,bmap,buff2);
    inc(dev,1);
}

// decrement number of free inodes or blocks
// blktype: 0 - inode, 1 - block
int dec(int dev, int blktype){
    get_block(dev, SBLK, buff);
    sp = (SUPER *)buff;
    if(blktype == 0)
        sp->s_free_inodes_count--;
    else if(blktype == 1)
        sp->s_free_blocks_count--;
    put_block(dev,SBLK,buff);
    get_block(dev,GDBLK,buff);
    gp = (GD *)buff;
    if(blktype == 0)
        gp->bg_free_inodes_count--;
    else if(blktype == 1)
        gp->bg_free_blocks_count--;
    put_block(dev,GDBLK,buff);
}

int my_rmdir(char *pathname){
    if(!pathname){
        printf("Please provide a pathname for the directory to remove\n");
        return -1;
    }
    int count, pino;
    // get the in-memory inode of the pathname
    int ino = getino(pathname);
    printf("my_rmdir(): dir to remove found at ino %d\n",ino);
    MINODE *mip = iget(dev,ino);
    INODE *ip = &mip->INODE;
    // verify the directory to remove is empty
    if(S_ISDIR(ip->i_mode) && mip->refCount == 1){
        for(int i = 0; i < 12; i++){
            count = 0;
            if(ip->i_block[i]){
                if(ip->i_links_count > 2){
                    printf("unable to remove dir: not empty\n");
                    return -1;
                }
                int blk = ip->i_block[i];
                get_block(dev,blk,buff);
                char *cp;
                DIR *dp = (DIR *)buff;
                cp = buff2;
                while(cp < buff + BLKSIZE){
                    count++;
                    cp += dp->rec_len;
                    dp = (DIR *)cp;
                }
                if(count > 2){
                    printf("unable to remoe dir: not empty\n");
                    return -1;
                }
            }
        }
    }
    printf("my_rmdir(): dir is empty, OK to remove\n");
    char parPath[128], dirName[128];
    strcpy(parPath,pathname);
    strcpy(dirName, pathname);
    char *par = dirname(parPath);
    printf("my_rmdir(): parent: %s\n",par);
    char *dir = basename(dirName);
    printf("my_dir(): dir: %s\n",dir);
    if(strcmp(par,"/")==0){
        pino = 2;
    }
    else if(strcmp(par,".")==0){
        pino = running->cwd->ino;
    }
    else{
        pino = getino(par);
    }
    // remove the dir from parent's data block
    MINODE *pmip = iget(dev,pino);

    rm_child(pmip, dir);
    bdalloc(dev,ip->i_block[0]);
    idalloc(dev,mip->ino);
    iput(mip);

    pmip->INODE.i_links_count--;
    pmip->dirty = 1;
    iput(pmip);
}

int rm_child(MINODE *pmip, char *name){
    int i,blk;
    int found = 0;
    DIR *dp,*dpend,*prev;
    char *cp, *cpend;
    INODE *ip = &pmip->INODE;

    // find the dir to remove in the DIRECT blocks
    for(i = 0; i < 12; i++){
        blk = ip->i_block[i];
        if(blk){
            printf("rm_child(): dir to remove in blk %d\n",blk);
            get_block(dev,blk,buff);
            dp = (DIR *)buff;
            cp = buff;
            // traverse through to find the dir to remove
            while(cp < buff + BLKSIZE){
                char dir[128];
                strcpy(dir,dp->name);
                dir[dp->name_len]=0;
                printf("name: %s dirname: %s\n",name,dir);
                if(strcmp(name, dir)==0){
                    printf("rm_child(): found dir to remove in the data block\n");
                    found = 1;
                    break;
                }
                cp += dp->rec_len;
                prev = dp;
                dp = (DIR *)cp;
            }
            if(found)
                break;
        }
    }
    if(!found){
        printf("rm_child(): dir not found to remove\n");
        return -1;
    }

    // dir has been found to remove, dp points to it
    // case dir is first item and only item in the data block:
    if(cp == buff && cp + dp->rec_len == buff + BLKSIZE){
        printf("rm_child(): removing dir as first and only item in block\n");
        bdalloc(dev,ip->i_block[i]);
        ip->i_size -= BLKSIZE;
        // compact the parent's i_block[] 
        while(ip->i_block[i+1] && i+1 < 12){
            i++;

            get_block(dev,ip->i_block[i],buff);
            put_block(dev,ip->i_block[i-1],buff);
        }
    }
    // if entry is the very last entry
    else if(cp + dp->rec_len == buff + BLKSIZE){
        printf("rm_child(): dir to remove is at end\n");
        // take add one file size to previous dir
        prev->rec_len += dp->rec_len;
        put_block(dev,ip->i_block[i],buff);
    }
    // entry is in the middle
    else{
        printf("rm_child(): found dir to remove in middle\n");
        // get the last dir
        get_block(dev,blk,buff);
        dpend = (DIR *)buff;
        cpend = buff;
        while(cpend + dpend->rec_len < buff + BLKSIZE){
            cpend += dpend->rec_len;
            dpend = (DIR *)cpend;
        }
        // adding the removed dir size to the last dir size
        dpend->rec_len += dp->rec_len;

        // move proceeding entries left
        unsigned char *start = cp+dp->rec_len;
        unsigned char *end = buff + BLKSIZE;
        memmove(cp,start,end-start);

        put_block(dev,ip->i_block[i],buff);
    }

    pmip->dirty=1;
    iput(pmip);
}

int quit(){
    for(int i = 0 ; i < NMINODE; i++){
        if(minodes[i].refCount > 0){
            printf("putting inode..\n");
            iput(&minodes[i]);
        }
    }
    printf("%s\n","Goodbye!\n");
}

int main(int argc, char *argv[]){
    init();
    mount_root();

    while(1){
        printf("Enter a command (cd | pwd | ls | mkdir | creat | quit): ");
        fgets(cmd,CMD_BUFF,stdin);
        int n = strlen(cmd);
        cmd[n-1] = 0;
        strcpy(cmdbuff,cmd);
        tokenizeCmd(cmdbuff);

        int c = findCmd(myargv[0]);

        switch(c){
            case -1:
                printf("invalid command\n");
                break;
            case 0: // cd
                cd(myargv[1]);
                break;
            case 1: // pwd
                pwd(running->cwd);
                break;
            case 2: // ls
                ls(myargv[1]);
                break;
            case 3: // mkdir
                my_mkdir(myargv[1]);
                break;
            case 4: // rmdir
                my_rmdir(myargv[1]);
                break;
            case 5: // creat
                printf("calling my_creat\n");
                my_creat(myargv[1]);
                break;
            case 6: // quit
                quit();
                exit(1);
                break;
        }
    }

    return 0;
}