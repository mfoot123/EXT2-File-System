#include "type.h"

/*ASSUME: file is opened for RD or RW;  e.g. fd = 0 opened for READ*/
int read_file()
{
    // Variables
    int fd, nbytes;
    char buf[BLKSIZE];

    // ask for a fd  and  nbytes to read;    e.g. read 0 10; read 0 123
    printf("Enter fd and number of bytes to read: ");
    scanf("%d %d", &fd, &nbytes);

    // verify fd is indeed opened for RD or RW;
    if (running->fd[fd]->mode != READ && running->fd[fd]->mode != READ_WRITE) {
        printf("error: fd is not open for RD or RW\n");
        return -1;
    }

    // return myread(fd, buf, nbytes);
    return myread(fd, buf, nbytes);
}



int myread(int fd, char *buf, int nbytes)
{

}