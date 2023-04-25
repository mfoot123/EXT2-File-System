#include "type.h"

int write_file() 
{
    int fd, nbytes;
    char buf[BLKSIZE];

    // ask for a fd   and   a text string to write;
    printf("Enter a file descriptor and a test string to write: ");
    scanf("%d %s", &fd, buf);

    // 2. verify fd is indeed opened for WR or RW or APPEND mode
    if (running->fd[fd]->mode != WRITE && running->fd[fd]->mode != READ_WRITE && running->fd[fd]->mode != APPEND) {
        printf("error: provided fd is not open for writing\n");
        return -1;
    }

    // 3. copy the text string into a buf[] and get its length as nbytes.
    nbytes = strlen(buf);

    // return mywrite(fd, buf, nbytes);
    return mywrite(fd, buf, nbytes);
}


