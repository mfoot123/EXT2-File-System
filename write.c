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



int mywrite(int fd, char buf[], int nbytes) 
{
    int ibuf[256];
    int *i, *j;
    int written = 0;

    // get the running PROC
    PROC *running = &proc[running->pid];

    // check if fd is valid and open for writing
    if (fd < 0 || fd >= NFD || running->fd[fd] == NULL || running->fd[fd]->mode == 0) {
        printf("mywrite : ERROR fd is invalid or not open for writing\n");
        return -1;
    }

    // get the OFT and MINODE pointer
    OFT *oftp = running->fd[fd];
    MINODE *mip = oftp->inodeptr;
    INODE *ip = &mip->INODE;

    // get the starting position for writing
    int lbk, startByte, blk;
    int remain, cq = 0;
    char wbuf[BLKSIZE];

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
            get_block(mip->dev, ip->i_block[12], (char *) ibuf);
            // blk = ibuf[lbk - 12];
            blk = ibuf[lbk - 12];

            if (blk == 0) {
                // allocate a disk block;
                blk = balloc(mip->dev);

                // record it in ibuf[lbk - 12];
                ibuf[lbk - 12] = blk;
            }
            // write ibuf[ ] back to disk block i_block[12];
            put_block(mip->dev, ip->i_block[12], (char *) ibuf);
        }
        // double indirect blocks
        else
        {
        // if no double indirect block yet, allocate one and zero it out
            if (ip->i_block[13] == 0) {
                // allocate a block for it;
                ip->i_block[13] = balloc(mip->dev);
                // zero out the block on disk !!!!
                get_block(mip->dev, ip->i_block[13], (char *) ibuf);
                bzero(ibuf, 0);
                put_block(mip->dev, ip->i_block[13], (char *) ibuf);
            }

            // get the double indirect block into a two-dimensional array
            int i, j, k;
            int dbuf[256];
            get_block(mip->dev, ip->i_block[13], (char *) dbuf);

            i = (lbk - (256 + 12)) / 256;
            j = (lbk - (256 + 12)) % 256;
            if (dbuf[i] == 0) {
                // allocate a single indirect block
                dbuf[i] = balloc(mip->dev);
                // zero out the block on disk !!!!
                get_block(mip->dev, dbuf[i], (char *) ibuf);
                bzero(ibuf, 0);
                put_block(mip->dev, dbuf[i], (char *) ibuf);
            }

            // get the single indirect block into an array
            get_block(mip->dev, dbuf[i], (char *) ibuf);
            k = lbk % 256;
            if (ibuf[j] == 0) {
                // allocate a data block
                ibuf[j] = balloc(mip->dev);
            }

            // write to the data block
            blk = ibuf[j];
            get_block(mip->dev, blk, wbuf);
            char *cp = wbuf + startByte;
            remain = BLKSIZE - startByte;
            while (remain > 0) {
                *cp++ = buf[cq++];
                nbytes--; remain--;
                oftp->offset++;
                if (oftp->offset > ip->i_size) {
                    mip->INODE.i_size++; 
                }
                if (nbytes <= 0) break;
            }
            put_block(mip->dev, blk, wbuf);

            // write the single indirect block back to disk
            put_block(mip->dev, dbuf[i], (char *) ibuf);

            // write the double indirect block back to disk
            put_block(mip->dev, ip->i_block[13], (char *) dbuf);
        }

        // read in the data block
        // read disk block into wbuf[ ]  
        get_block(mip->dev, blk, wbuf);
        // cp points at startByte in wbuf[]
        char *cp = wbuf + startByte;
        // number of BYTEs remain in this block
        remain = BLKSIZE - startByte;

        // write as much as remain allows 
        while (remain > 0) {
            // cq points at buf[ ]
            *cp++ = buf[cq++];
            // dec counts
            nbytes--; remain--;
            // // advance offset
            oftp->offset++;

            // especially for RW|APPEND mode
            if (oftp->offset > ip->i_size) {
                // inc file size (if offset > fileSize)
                mip->INODE.i_size++; 
            }
            // if already nbytes, break
            if (nbytes <= 0) break;
        }

        // write wbuf[ ] to disk
        put_block(mip->dev, blk, wbuf);

        if (nbytes <= 0) {
            break;
        }
        // loop back to outer while to write more .... until nbytes are written
    }

    // mark mip modified for iput() 
    mip->modified = 1;
    printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);           
    return nbytes;
}
