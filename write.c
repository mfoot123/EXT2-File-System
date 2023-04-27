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



int mywrite(int fd, char* buf, int nbytes)
{
    puts(buf);
    int ibuf[256];
    int *i, *j;
    int written = 0;

    // get the OFT and MINODE pointer
    OFT *oftp = running->fd[fd];
    MINODE *mip = oftp->inodeptr;
    INODE *ip = &mip->INODE;

    // get the starting position for writing
    int lbk, startByte, blk;
    int remain = 0;
    
    char wbuf[BLKSIZE];
    char* cq = buf;
    // while (nbytes > 0 ){
    while (nbytes > 0) {

        // compute LOGICAL BLOCK (lbk) and the startByte in that lbk:
        // lbk       = oftp->offset / BLKSIZE;
        lbk = oftp->offset / BLKSIZE;
        // startByte = oftp->offset % BLKSIZE;
        startByte = oftp->offset % BLKSIZE;

        // direct block
        if (lbk < 12) {
            // if no data block yet, allocate one
            if (ip->i_block[lbk] == 0) {
                ip->i_block[lbk] = balloc(mip->dev);
            }
            blk = ip->i_block[lbk];
        }
        // indirect blocks
        else if (lbk >= 12 && lbk < 256 + 12) {
            // if no indirect block yet, allocate one and zero it out
            if (ip->i_block[12] == 0) {
                // allocate a block for it;
                ip->i_block[12] = balloc(mip->dev);
                // zero out the block on disk !!!!
                get_block(mip->dev, ip->i_block[12], (char *) ibuf);
                bzero(ibuf, 0);
                put_block(mip->dev, ip->i_block[12], (char *) ibuf);    
            }

            // get i_block[12] into an int ibuf[256];
            get_block(mip->dev, ip->i_block[12], (char*) ibuf);
            // blk = ibuf[lbk - 12];
            blk = ibuf[lbk - 12];

            if (blk == 0) {
                // allocate a disk block;
                blk = balloc(mip->dev);

                // record it in ibuf[lbk - 12];
                ibuf[lbk - 12] = blk;
                put_block(dev, ip->i_block[12], ibuf);
            }
            // write ibuf[ ] back to disk block i_block[12];
            put_block(mip->dev, ip->i_block[12], (char *) ibuf);
        }
        // double indirect blocks
        else
        {
            int dibuf[256];
            // if no double indirect block yet, allocate one and zero it out
            if (ip->i_block[13] == 0) {
                // allocate a block for it;
                ip->i_block[13] = balloc(mip->dev);
                // zero out the block on disk !!!!
                get_block(mip->dev, ip->i_block[13], (char *) ibuf);
                bzero(ibuf, 0);
                put_block(mip->dev, ip->i_block[13], (char *) ibuf);
            }

            int i, j, k;
            get_block(mip->dev, ip->i_block[13], (char *) ibuf);

            i = (lbk - 12 - 256) / 256;
            j = (lbk - 12 - 256) % 256;
            if (ibuf[i] == 0) {
                // allocate a single indirect block
                ibuf[i] = balloc(mip->dev);
                // zero out the block on disk !!!!
                get_block(mip->dev, ibuf[i], (char *)wbuf);
                bzero(wbuf, 0);
                put_block(mip->dev, ibuf[i], (char *)wbuf);
                put_block(mip->dev, ip->i_block[13], ibuf);
            }

            get_block(mip->dev, ibuf[i], (char *) dibuf);
            if (dibuf[j] == 0) {
                // allocate a data block
                dibuf[j] = balloc(mip->dev);
                get_block(mip->dev, dibuf[j], wbuf);
                bzero(wbuf, 0);
                put_block(mip->dev, dibuf[j], wbuf);
                put_block(mip->dev, ibuf[i], dibuf);
            }

            // write to the data block
            blk = dibuf[j];
            // write the single indirect block back to disk
            put_block(mip->dev, ibuf[i], (char *) dibuf);

            // write the double indirect block back to disk
            put_block(mip->dev, ip->i_block[13], (char *) ibuf);
        }

        // read in the data block
        // read disk block into wbuf[ ]  
        get_block(mip->dev, blk, wbuf);
        // cp points at startByte in wbuf[]
        char *cp = wbuf + startByte;
        // number of BYTEs remain in this block
        remain = BLKSIZE - startByte;

        // write as much as remain allows 
            // cq points at buf[ ]
        strncpy(cp, cq, remain);
        cp += remain;
        cq += remain;
        written += remain;
        oftp->offset += remain;
        // dec counts
        nbytes -= remain;

        // // advance offset

        // especially for RW|APPEND mode
        // update file size if necessary
        if (oftp->offset > mip->INODE.i_size) {
            mip->INODE.i_size = oftp->offset;
        }

        // write wbuf[ ] to disk
        put_block(mip->dev, blk, wbuf);

        // loop back to outer while to write more .... until nbytes are written
    }
    
    // mark mip modified for iput() 
    mip->modified = 1;
    //printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);           
    return written;
}