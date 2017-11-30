#include "l7.h"
#include "utils.c"
#include "cd.c"
#include "mkdir.c"
#include "rmdir.c"
#include "pwd.c"
#include "ls.c"
#include "creat.c"
#include "link.c"
#include "unlink.c"
#include "symlink.c"
#include "readlink.c"
#include "bitmanip.c"
#include "cmd_proc.c"

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
// cmd args
char *myargv[64];

/////////////////////////////////////////////////////////////////////////
// quit() for any changes write it back into mydisk then quit.
/////////////////////////////////////////////////////////////////////////
int quit(){
    for(int i = 0 ; i < NMINODE; i++){
        if(minodes[i].refCount > 0){
            printf("putting inode..\n");
            iput(&minodes[i]);
        }
    }
    printf("%s\n","Goodbye!\n");
}

/////////////////////////////////////////////////////////////////////////
// main()
/////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){
    init();
    mount_root();

    while(1){
        printf("Enter a command (cd | pwd | ls | mkdir | creat | rmdir | link | unlink\n");
        printf("| symlink | readlink | quit): ");
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
            case 6: // link
                if(myargv[1] && myargv[2])
                    my_link(myargv[1],myargv[2]);
                else
                    printf("(HELP) link command: link old_file new_file\n");
                break;
            case 7: // unlink
                my_unlink(myargv[1]);
                break;
            case 8: // symlink
                if(myargv[1] && myargv[2])
                    my_symlink(myargv[1],myargv[2]);
                else
                    printf("(HELP) symlink command: symlink old_file new_file\n");
                break;
            case 9: // readlink
            {
                char *buffer = malloc(128*sizeof(char));
                if(my_readlink(myargv[1],buffer)>=0)
                    printf("link: %s\n",buffer);
                break;
            }
            case 10: // quit
                quit();
                exit(1);
                break;
        }
    }

    return 0;
}