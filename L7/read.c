#include "l7.h"

int my_read(int fd, char buf[], int nbytes, int space){
    if(running->fd[fd]->mode == READ_MODE || running->fd[fd]->mode == RW_MODE){
        
    }
    else{
        printf("Cannot read file. File was not opened for valid read mode\n");
        return -1;
    }
}

int read_file(int fd, char buf[], int nbytes, int space){

}