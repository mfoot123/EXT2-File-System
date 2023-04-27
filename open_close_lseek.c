#include "type.h"

/**********************************************************************
 * Function: 
 * Discription: 
 * Last Modified: 
 * Used in: 
 * References: 
 ***********************************************************************/
int open_file(const char *pathname, int mode)
{
    // Step 1
    if (mode < 0 || mode > 3) {
        printf("invalid mode\n");
        return -1;
    }

    // Step 2
    MINODE *mip = path2inode(pathname);

    // Step 3
    if (!mip) {
        if (mode == 0) {
            // READ: file must exist
            return -1;
        } else {
            // mode=R|RW|APPEND: creat file first
            creat_file(pathname);
            mip = path2inode(pathname);
            // print mip's [dev, ino]
        }
    }

    // Verify that the file is a regular file
    if (!S_ISREG(mip->INODE.i_mode)) {
        printf("file is not a regular file\n");
        iput(mip);
        return -1;
    }

    // Check if file is already opened with incompatible mode
    for (int i = 0; i < NFD; i++) {
        if (running->fd[i] && running->fd[i]->inodeptr == mip) {
            if (mode == 1 || mode == 2 || mode == 3) {
                printf("file is already opened with incompatible mode\n");
                iput(mip);
                return -1;
            }
        }
    }

    // Step 4: Allocate a FREE OpenFileTable (OFT) and fill in values
    int i;
    OFT *oftp = 0;
    for (i = 0; i < NFD; i++) {
        if (!running->fd[i]) {
            oftp = running->fd[i] = (OFT *)malloc(sizeof(OFT));
            break;
        }
    }
    if (!oftp) {
        printf("no available file descriptors\n");
        iput(mip);
        return -1;
    }
    oftp->mode = mode;
    oftp->shareCount = 1;
    oftp->inodeptr = mip;

    // Step 5: Set the OFT's offset accordingly
    switch (mode) {
        case 0: // R: offset = 0
            oftp->offset = 0;
            break;
        case 1: // W: truncate file to 0 size
            truncate(mip);
            oftp->offset = 0;
            break;
        case 2: // RW: do not truncate file
            oftp->offset = 0;
            break;
        case 3: // APPEND mode
            oftp->offset = mip->INODE.i_size;
            break;
    }

    // Step 8: Update INODE's time field
    time_t now = time(0L);
    mip->INODE.i_atime = now;
    if (mode != 0) {
        mip->INODE.i_mtime = now;
    }
    mip->modified = 1;

    // Return the file descriptor
    return i;
}


/**********************************************************************
 * Function:
 * Discription:
 * Last Modified:
 * Used in:
 * References:
 ***********************************************************************/
int myTruncate(MINODE *mip)
{
//   1. release mip->INODE's data blocks;  // SAME as print data blocks in LAB6
    
//      a file may have  12 direct blocks, 
//                      256 indirect blocks and 
//                      256*256 double indirect data blocks. 
//      release them all.

//   2. update INODE's time fields

//   3. set INODE's size to 0 and mark minode MODIFIED
    return 0;
}

/**********************************************************************
 * Function:
 * Discription:
 * Last Modified:
 * Used in:
 * References:
 ***********************************************************************/
int close_file(int fd)
{
    // 1. verify fd is within range.
    if (fd < 0 || fd > 15)
    { // Check if fieDiscriptor is within range
        printf("File Discriptor is not within range");
        return -1;
    }
    printf("File is within range\n");

    // 2. verify running->fd[fd] is pointing at a OFT entry
    if(running->fd[fd] == NULL){
        printf("ERROR: not pointing at oft entry");
        return -1;
    }

// 3. The following code segments should be obvious:
     OFT *oftp = running->fd[fd];
     running->fd[fd] = 0;
     oftp->shareCount--;
     if (oftp->shareCount > 0)
        return 0;

    // last user of this OFT entry ==> dispose of its minode
    MINODE *mip = oftp->inodeptr;
    iput(mip);
    printf("File Discriptor %d closed\n", fd);
    return 0;
}

/**********************************************************************
 * Function:
 * Discription:
 * Last Modified:
 * Used in:
 * References:
 ***********************************************************************/
int myLSeek(int fd, int position)
{
//       From fd, find the OFT entry. 
    if (fd == NULL){ //Check if fileDiscriptor is not null
        return -1;
    }
    if(fd < 0|| fd > 15 ){ //Check if fieDiscriptor is within range
        return -1;
    }

    OFT *oft = running->fd[fd];
    if (oft == NULL)
    {
        return -1;
    }
    int originalPosition = oft->offset;
    oft->offset = position;

    //   change OFT entry's offset to position but make sure NOT to over run either end
    //   of the file.

    //   return originalPosition
    return originalPosition;
}

/**********************************************************************
 * Function:
 * Discription:
 * Last Modified:
 * Used in:
 * References:
 ***********************************************************************/
int pfd()
{
//       This function displays the currently opened files as follows:

//         fd     mode    offset    INODE
//        ----    ----    ------   --------
//          0     READ    1234   [dev, ino]  
//          1     WRITE      0   [dev, ino]
//       --------------------------------------
//   to help the user know what files has been opened.
    char* temp = "\0";
    printf("fd\tmode\toffset\tINODE [dev, ino]\n");
    for (int i = 0; i < NFD; i++)
    {
        if (running->fd[i] == NULL)
            break;

        if (oft[i].mode == READ)
        {
            temp = "READ";
        }
        else if(oft[i].mode == WRITE){
            temp = "WRITE";
        }
        else if (oft[i].mode == READ_WRITE)
        {
            temp = "READ_WRITE";
        }
        else
        {
            temp = "APPEND";
        }
        printf("%d\t%s\t%d\t[%d, %d]\n", i, temp, oft[i].offset, oft[i].inodeptr->dev, oft[i].inodeptr->ino);
    }
    return 0;
}

/**********************************************************************
 * Function:
 * Discription:
 * Last Modified:
 * Used in:
 * References:
 ***********************************************************************/
int dup(int fd){
    //   verify fd is an opened descriptor;
    fd = open_file(pathname, READ);

    if(fd < 0){
        printf("File Discriptor is not open");
        return -1;
    }
        //   duplicates (copy) fd[fd] into FIRST empty fd[ ] slot;
    for (int i = 0; i < NFD; i++)//Need to allocate and empty fd slot
    {
        if (oft[i].shareCount == 0)
        {
            oft[i].mode = running->fd[fd]->mode; // mode = 0|1|2|3 for R|W|RW|APPEND (swapped with mode)
            oft[i].inodeptr = running->fd[fd]->inodeptr; // point at the file's minode[]
            oft[i].offset = running->fd[fd]->offset;
            oft[i].shareCount = 1; // increment OFT's shareCount by 1;
            printf("Successfully copied");
            return 0;
        }
    }
    
    return 0;
}

/**********************************************************************
 * Function:
 * Discription:
 * Last Modified:
 * Used in:
 * References:
 ***********************************************************************/
int dup2(int fd, int gd)
{
    // CLOSE gd fisrt if it's already opened;
    gd = open_file(pathname, READ);
    if (gd < 0)
    {
        printf("Group Discriptor is open");
        close_file(gd);
    }

//   duplicates fd[fd] into fd[gd];
    running->fd[gd]->mode = running->fd[fd]->mode;         // mode = 0|1|2|3 for R|W|RW|APPEND (swapped with mode)
    running->fd[gd]->inodeptr = running->fd[fd]->inodeptr; // point at the file's minode[]
    running->fd[gd]->offset = running->fd[fd]->offset;
    oft[gd].shareCount = 1; // increment OFT's shareCount by 1;
    printf("Successfully copied");
    return 0;
}