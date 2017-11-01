#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#include "utils.c"

#define CMD_BUFF 128

// possible commands
char *cmds[] = {"cd","pwd","ls","mkdir","quit"};
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

// pwd() utility function
int rpwd(MINODE *wd){
    printf("rpwd(): getting working directory...\n");
    int my_ino, par_ino;
    char dirname[64];
    printf("rpwd(): before root check\n");
    if(wd == root){
        return;
    }
    printf("rpwd(): after root check\n");
    INODE *tip = &wd->INODE;
    // get my_ino and parent_ino
    my_ino = wd->ino;
    printf("rpwd(): my_ino is %d\n",my_ino);
    int blk = tip->i_block[0];
    get_block(dev,blk,buff);
    // get to parent_ino location
    DIR *dp = (DIR *)buff;
    char *cp = buff;
    cp += dp->rec_len;
    dp = (DIR *)cp;

    par_ino = dp->inode;
    printf("rpwd(): par_ino is %d\n",par_ino);

    // get parent inode to get name of wd
    MINODE *pmip = iget(dev,par_ino);
    tip = &pmip->INODE;
    blk = tip->i_block[0];
    get_block(dev,blk,buff);
    dp = (DIR *)buff;
    cp = buff;
    printf("rpwd(): iterating dirs...\n");
    while(cp < buff + BLKSIZE){
        if(dp->inode == my_ino){
            strcpy(dirname,dp->name);
            printf("rpwd(): dirname is %s\n",dirname);
            break;
        }
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    printf("rpwd(): making recursive call\n");
    rpwd(pmip);
    printf("/%s\n",dirname);
}

// prints the working directory
int pwd(MINODE *wd){
    if(wd == root){
        printf("current working directory: /\n");
    }
    else{
        rpwd(wd);
    }
}

// change cwd to pathname
int cd(char *pathname){
    int ino;
    printf("cd(): changing directory...\n");
    if(!pathname){
        // cd to root
        ino = getino("/");
    }else{
        printf("cd(): getting ino...\n");
        ino = getino(pathname);
        printf("cd(): ino found at %d",ino, running->cwd->ino);
    }
    MINODE *mip = iget(dev,ino);
    if(S_ISDIR(mip->INODE.i_mode)){
        printf("cd(): inode is DIR...\n");
        iput(running->cwd);
        running->cwd = mip;
        printf("cd(): running->cwd changed\n");
        return ino;
    }
}

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
    char last_mod[64];
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
    if((ip->i_mode & 0xF000)==0x8000){
        printf("%3s","r");
    }
    else if((ip->i_mode & 0xF000)==0x4000){
        printf("%3s","d");
    }
    else{
        printf("%3s","l");
    }

    // uid and gid
    printf("%3d%3d%8d",ip->i_uid,ip->i_gid,ip->i_size);

}

// test bit value
int test_bit(char *buf, int bit){
    return buf[bit/8] & (1 << (bit % 8));
}

// set bit value
int set_bit(char *buf, int bit){
    buf[bit/8] |= (1 << (bit % 8));
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

// allocates an inode on device
int ialloc(int dev){
    char buf[BLKSIZE];
    // get imap block
    get_block(dev,imap,buf);
    for(int i = 0; i < ninodes; i++){
        if(test_bit(buf,i)==0){
            set_bit(buf,i);
            put_block(dev,imap,buff);
            dec(dev,0);
            return (i+1); // +1 to go back to 1 index for reuse in iget
        }
    }
    return 0; // no free inodes available
}

// allocates a free disk block on device
int balloc(int dev){
    char buf[BLKSIZE];
    // get block bitmap
    get_block(dev,bmap,buf);
    for(int i = 0; i < nblocks;i++){
        if(test_bit(buf,i)==0){
            set_bit(buf,i);
            put_block(dev,bmap,buff);
            dec(dev,1);
            return (i+1); // +1 to go back to 1 index for reuse in iget
        }
    }
    return 0; // no block available
}

// enters a new dir into the parent directory
int enter_child(MINODE *pip,int ino, char *child){
    DIR *dp;
    char *cp;
    int ideal_len, remain, blk, i;
    int need_len = 4*((8+strlen(child)+3)/4);

    for(i = 0; i < 12; i++){
        blk = pip->INODE.i_block[i];
        if(blk == 0)
            break;
        get_block(dev,blk,buff);
        dp = (DIR *)buff;
        cp = buff;
        while(cp + dp->rec_len < buff + BLKSIZE){
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
        // dp points at last entry in parent's data block
        // get the amount of remaining space in the block
        ideal_len = 4*((8+dp->name_len+3)/4);
        remain = dp->rec_len - ideal_len;
        // add the new entry as the last entry
        if(remain >= need_len){
            dp->rec_len = ideal_len;

            cp += dp->rec_len;
            dp = (DIR *)cp;

            dp->inode = ino;
            dp->rec_len = (BLKSIZE - (cp - buff));
            dp->name_len = strlen(child);
            dp->file_type = (char)EXT2_FT_DIR;
            strcpy(dp->name,child);

            // write the block back to disk
            put_block(dev, blk, buff);
            return 1;
        }
    }

    // no space left in block, allocate new one
    blk = balloc(dev);
    pip->INODE.i_block[i] = blk;
    pip->INODE.i_size += BLKSIZE;
    pip->dirty = 1;

    get_block(dev,blk,buff);
    dp = (DIR *)buff;
    cp = buff;

    dp->inode = ino;
    dp->rec_len = BLKSIZE;
    dp->name_len = strlen(child);
    dp->file_type = (char)EXT2_FT_DIR;
    strcpy(dp->name,child);

    put_block(dev,blk,buff);
    return 1;
}

int my_mkdir(MINODE *pmip, char *dir){
    int ino = ialloc(dev);
    int blk = balloc(dev);

    // create the new inode
    MINODE *mip = iget(dev,ino); // load the newly allocated inode into mem
    INODE *ip = &mip->INODE;
    ip->i_mode = 0x41ED; // DIR type & permissions
    ip->i_uid = running->uid; // owner uid
    ip->i_gid = running->pid; // group id
    ip->i_size = BLKSIZE;
    ip->i_links_count = 2;
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 2;
    ip->i_block[0] = blk;
    for(int i = 1; i < 15;i++){
        ip->i_block[i] = 0;
    }
    mip->dirty = 1;
    iput(mip);

    // create the . and .. dirs for the new inode
    char buf[BLKSIZE];
    bzero(buf,BLKSIZE);
    DIR *dp = (DIR *)buf;
    // make the . entry
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';
    // make the .. entry
    dp = (char *)dp + 12;
    dp->inode = pmip->ino;
    dp->rec_len = BLKSIZE-12;
    dp->name_len = 2;
    dp->name[0] = dp->name[1] = '.';
    put_block(dev,blk,buf);

    // enter the newly created dir as a dir of the parent
    enter_child(pmip,ino,dir);
}

// TODO: mkdir()
int mkdir(char *pathname){
    printf("mkdir(): making dir...\n");
    char *pathCpy;
    strcpy(pathCpy,pathname);

    char *parent = dirname(pathCpy);
    char *dir = basename(pathCpy);
    printf("mkdir(): parent: %s\n",parent);
    printf("mkdir(): dir: %s\n",dir);

    int pino;
    if(strcmp(parent,"/")==0){
        pino = 2;
    }
    else{
        pino = getino(parent);
    }

    MINODE *pmip = iget(dev,pino);
    INODE *ip = &pmip->INODE;
    if(S_ISDIR(ip->i_mode)){
        int isFound = search(pmip,dir);
        if(isFound == -1){
            my_mkdir(pmip,dir);
        }
    }
    pmip->INODE.i_links_count++;
    pmip->dirty=1;
}

// TODO: creat()
int creat(char *filename){

}

// TODO: rmdir()
int rmdir(char *pathname){

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
        printf("Enter a command (cd | pwd | ls | mkdir | quit): ");
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
                mkdir(myargv[1]);
                break;
            case 4: // quit
                quit();
                exit(1);
                break;
        }
    }

    return 0;
}