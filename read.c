#include "type.h"

/*ASSUME: file is opened for RD or RW;  e.g. fd = 0 opened for READ*/
int read_file()
{
    // Variables
    int fd, nbytes;
    char buf[BLKSIZE];

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
    int count = 0;
    OFT *oftp = running->fd[fd];
    MINODE *mip = oftp->inodeptr;
    int fileSize = mip->INODE.i_size;
    int avil = fileSize - oftp->offset; // number of bytes still available in file
    char *cq = buf; // cq points at buf[ ]
    char ibuf[BLKSIZE];

    while (nbytes && avil) {

        // Compute LOGICAL BLOCK number lbk and startByte in that block from offset
        int lbk = oftp->offset / BLKSIZE;
        int startByte = oftp->offset % BLKSIZE;

        int blk;
        char readbuf[BLKSIZE];

        // If the logical block number is less than 12, it's a direct block
        if (lbk < 12) {
            blk = mip->INODE.i_block[lbk];
        }
        // If the logical block number is between 12 and 12 + 256 - 1, it's an indirect block
        else if (lbk >= 12 && lbk < 12 + 256) {
            // Compute the logical block number relative to the start of the indirect blocks
            int indirect_lbk = lbk - 12;
            // Load the indirect block
            get_block(mip->dev, mip->INODE.i_block[12], readbuf);
            // Get the physical block number from the indirect block
            blk = ibuf[indirect_lbk];
        }
        // If the logical block number is greater than or equal to 12 + 256, it's a double indirect block
        else {
            char dibuf[256];
            char idbuf[256];
            // Compute the logical block number relative to the start of the double indirect blocks
            lbk = (lbk - 12 - 256) % 256;
            // Load the double indirect block
            get_block(mip->dev, mip->INODE.i_block[13], dibuf);
            // Get the physical block number of the indirect block from the double indirect block
            int indirect_blk = ((int *)readbuf)[lbk / 256];
            // Load the indirect block
            get_block(mip->dev, indirect_blk, readbuf);
            // Get the physical block number from the indirect block
            blk = ibuf[lbk];
        }

        // Load the data block into readbuf[BLKSIZE]
        get_block(mip->dev, blk, readbuf);

        // Copy from startByte to buf[], at most remain bytes in this block
        char *cp = readbuf + startByte;
        int remain = BLKSIZE - startByte; // number of bytes remaining in readbuf[]

        while (remain > 0) 
        {
            // check for full block
            if(remain < avil)
            {
                // dec by remain
                strncpy(cq, cp, remain); // copy byte from readbuf[] into buf[]
                oftp->offset += remain; // advance offset
                count += remain; // increment count as number of bytes read
                avil -= remain;
                nbytes -= remain;
                remain -= remain;
            }
            else
            {
                // dec by avil
                strncpy(cq, cp, avil); // copy byte from readbuf[] into buf[]
                oftp->offset += avil; // advance offset
                count += avil; // increment count as number of bytes read
                remain -= avil;
                nbytes -= avil;
                avil -= avil;
            }

            if (nbytes <= 0 || avil <= 0) {
                break;
            }
        }

        // If one data block is not enough, loop back to OUTER while for more
    }

    printf("myread: read %d char from file descriptor %d\n", count, fd);

    return count; // count is the actual number of bytes read
}






