#include "l7.h"

/////////////////////////////////////////////////////////////////////////
// my_link() creates a hard link between two files
/////////////////////////////////////////////////////////////////////////
int my_link(char *old_file, char *new_file){
    printf("link(): linking file %s to %s\n",new_file,old_file);
    char path_buff[128], file_buff[128];
    strcpy(path_buff,new_file);
    strcpy(file_buff,new_file);
    // get the inode to hard link to
    int oldino = getino(old_file);
    if(oldino == 0){
        printf("Unable to link, old file does not exist!\n");
    }
    printf("link(): linking to ino: %d\n",oldino);
    MINODE *oldmip = iget(dev,oldino);
    // check we are hardlinking to file
    if(S_ISREG(oldmip->INODE.i_mode)){
        printf("link(): creating new file...\n");
        // create new file to hardlink
        char *parent = dirname(path_buff);
        char *file = basename(file_buff);
        printf("link(): file to be made: %s\n",file);
        int parentino = getino(parent);
        MINODE *parentmip = iget(dev,parentino);
        // check that new file does not exist
        if(search(parentmip,file)<0){
            enter_child(parentmip,oldino,file);
            oldmip->INODE.i_links_count++;
            oldmip->dirty=1;
            iput(oldmip);
            iput(parentmip);
        }
    }
    else{
        printf("Cannot link to DIR type!\n");
    }
}