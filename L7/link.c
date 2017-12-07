#include "l7.h"
/*
********* Algorithm of link ********
link(old_file, new_file)
{
    1. // verify old_file exists and is not DIR;
    oino = getino(&odev, old_file);
    omip = iget(odev, oino);
    check file type (cannot be DIR).
    2. // new_file must not exist yet:
    nion = get(&ndev, new_file) must return 0;
    ndev of dirname(newfile) must be same as odev
    3. // creat entry in new_parent DIR with same ino
    pmip -> minode of dirname(new_file);
    enter_name(pmip, omip->ino, basename(new_file));
    4. omip->INODE.i_links_count++;
    omip->dirty = 1;
    iput(omip);
    iput(pmip);
}
*/

/////////////////////////////////////////////////////////////////////////
// my_link() link(old_file, new_file) creates a hard link from new_file to old_file
// return: none
// Hard links can only be to regular files, not DIRs, 
// because linking to DIRs may create loops in the file system name space. Hard
// link files share the same inode. Therefore, they must be on the same device.
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