#include "l7.h"

// possible commands
char *cmds[] = {"cd","pwd","ls","mkdir","rmdir","creat","link","unlink","symlink"
,"readlink","chmod","touch","open","close","read","write","quit"};
// user input (command, command buffer)
char cmd[CMD_BUFF], cmdbuff[CMD_BUFF];
char *myargv[64];
char *names[64];

/////////////////////////////////////////////////////////////////////////
// my_close() tokenizes user input into command and params
// return: none
/////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////
// findCmd() finds command entered by user
// return: index of commande array, -1 for not find command
/////////////////////////////////////////////////////////////////////////
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