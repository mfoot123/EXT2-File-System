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
    // variables:
    int count = 0;
    int avil, blk, remain;
    char *cq = buf;
    char readbuf[BLKSIZE];

    // get offset from file descriptor
    int offset = running->fd[fd]->offset;
    // get fileSize from the i_size of the INODE of inodeptr
    int fileSize = running->fd[fd]->inodeptr->INODE.i_size;

    // calculate avil
    avil = fileSize - offset;

    while (nbytes && avil) 
    {
        // Compute LOGICAL BLOCK number lbk and startByte in that block from offset;
        int lbk = offset / BLKSIZE;
        int startByte = offset % BLKSIZE;
        
        // Map logical block to physical block
        if (lbk < 12) {  // direct blocks
            blk = running->fd[fd]->inodeptr->INODE.i_block[lbk];
        } else if (lbk >= 12 && lbk < 256 + 12) {  // indirect blocks
            // get the block number of the indirect block
            get_block(running->fd[fd]->inodeptr->dev, running->fd[fd]->inodeptr->INODE.i_block[12], readbuf);
            int *indirect = (int *)readbuf;
            blk = indirect[lbk - 12];
        } else {  // double indirect blocks
            // get the block number of the double indirect block
            get_block(running->fd[fd]->inodeptr->dev, running->fd[fd]->inodeptr->INODE.i_block[13], readbuf);
            int *indirect2 = (int *)readbuf;
            // get the block number of the indirect block
            get_block(running->fd[fd]->inodeptr->dev, indirect2[(lbk - 256 - 12) / 256], readbuf);
            int *indirect = (int *)readbuf;
            // get the block number of the data block
            blk = indirect[(lbk - 256 - 12) % 256];
        }
        
        // get the data block into readbuf[BLKSIZE]
        get_block(running->fd[fd]->inodeptr->dev, blk, readbuf);
        
        // copy from startByte to buf[ ], at most remain bytes in this block
        char *cp = readbuf + startByte;
        remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]
        
        while (remain > 0 && nbytes > 0 && avil > 0) {
            *cq++ = *cp++;             // copy byte from readbuf[] into buf[]
            offset++;           // advance offset 
            count++;                  // inc count as number of bytes read
            avil--; nbytes--;  remain--;
            if (nbytes <= 0 || avil <= 0) break;  // finished reading requested bytes
        }

        // if one data block is not enough, loop back to OUTER while for more ...

    }
    
    printf("myread: read %d bytes from file descriptor %d\n", count, fd);
    
    // updates offset
    running->fd[fd]->offset = offset;
    return count;
}



