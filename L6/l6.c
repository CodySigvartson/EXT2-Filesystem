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
char *cmds[] = {"cd","pwd","ls","quit"};
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
    char *dirname = malloc(sizeof(char) * 64);
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
            dirname = dp->name;
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

int quit(){
    printf("%s\n","Goodbye!\n");
}

int main(int argc, char *argv[]){
    init();
    mount_root();

    while(1){
        printf("Enter a command (cd | pwd | ls | quit): ");
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
            case 3: // quit
                quit();
                exit(1);
                break;
        }
    }

    return 0;
}