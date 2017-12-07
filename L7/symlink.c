#include "l7.h"

/*

Algorithm of symlink(old_file, new_file)
{
    1. check: old_file must exist and new_file not yet exist;
    2. create new_file; change new_file to SLINK type;
    3. // assume length of old_file name <= 60 chars
    store old_file name in newfile's INODE.i_block[ ] area.
    mark new_file's minode dirty;
    iput(new_file's minode);
    4. mark new_file parent minode dirty;
    put(new_file's parent minode);
}

*/

/////////////////////////////////////////////////////////////////////////
// my_symlink_util() 
// return: none
/////////////////////////////////////////////////////////////////////////
int my_symlink_util(MINODE *pmip,char *old_file, char *file){
    // allocate a location for the new file
    int ino = ialloc(dev);

    // create the new inode
    MINODE *mip = iget(dev,ino); // load the newly allocated inode into mem
    INODE *ip = &mip->INODE;
    ip->i_mode = (unsigned short)0xA1A4; // REG type & permissions
    ip->i_uid = (unsigned short)running->uid; // owner uid
    ip->i_gid = (unsigned short)running->pid; // group id
    ip->i_size = strlen(old_file);
    ip->i_links_count = (unsigned short)1;
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 0;
    memcpy(ip->i_block,old_file,sizeof(old_file));
    mip->dirty = 1;
    iput(mip);

    // enter the newly created file as a file of the parent
    enter_child(pmip,ino,file);
}

/////////////////////////////////////////////////////////////////////////
// my_symlink() link from new_file to old_file. Unlike
//                   hard links, symlink can link to anything, 
//                   including DIRs or files not on the same device
// return: none
/////////////////////////////////////////////////////////////////////////
int my_symlink(char *old_file, char *new_file){
    char path_buff[128], file_buff[128];
    strcpy(path_buff,new_file);
    strcpy(file_buff,new_file);
    // get the parent path of the new file being made 
    char *parent = dirname(path_buff);
    char *file = basename(file_buff);
    // get the parent inode into mem
    int pino;
    if(strcmp(parent,"/")==0){
        pino = 2;
    }
    else if(strcmp(parent,".")==0){
        pino = running->cwd->ino;
    }
    else{
        pino = getino(parent);
    }
    MINODE *pmip = iget(dev,pino);

    my_symlink_util(pmip,old_file,file);
     
    pmip->dirty=1;
    iput(pmip);
}