#include "type.h"

/**********************************************************************
 * Function: 
 * Discription: 
 * Last Modified: 
 * Used in: 
 * References: 
 ***********************************************************************/
int open_file(char* pathname, int mode)// have arguements
{
    printf("Tokenize: echo names: %s\n", pathname);
    char sbuf[BLKSIZE];

    // 1. ASSUME open pathname mode   # mode = 0|1|2|3 for R|W|RW|APPEND
    //int mode = running->fd[fd]->mode;

    // 2. get pathname's minode pointer:
    MINODE *mip = path2inode(pathname);

    // 3. If path currently does not exist
    if (!mip) // If Pathname does not exist
    {
        printf("ERROR: mip does not exists");
        if (mode == READ) // If READ: file must exist
        {
            printf("ERROR: Mode is on read");
            return -1;
        }
        creat_file(pathname); // creat(); make sure creat_file() uses pathname
        mip = path2inode(pathname); //Repeating step 2
    }

    printf("Search for %s in inode# %d\n", pathname, mip->id);
    get_block(dev, mip->INODE.i_block[0], sbuf);
    printf("i_block[0] = %d\n", mip->INODE.i_block[0]);
    show_dir(mip); // print mips[dev, ino]//////////////////////////////////////////////Issue begins here

    // 4. check mip->INODE.i_mode to verify it's a REGULAR file
    if (!S_ISREG(mip->INODE.i_mode)){
        printf("ERROR: does not work\n");
        return -1;
    }

    // Check whether the file is ALREADY opened with INCOMPATIBLE mode:
    // If it's already opened for W, RW, APPEND : reject.
    // that is, only multiple READs of the SAME file are OK


    // we need to search the entire table for an fd with the same inode number w/ incompaitble mode
    // loop through j < NFD
    for(int j = 0; j < NFD; j++)
    {
        // make sure running->fd == mip->ino
        if(running->fd[j] && running->fd[j]->inodeptr->ino == mip->ino)
        {
            //running->fd.mode != 0
            if(running->fd[j]->mode != 0)
            {
                // error
                printf("Error: File is open for Read Write or Append\n");
                return -1;
            }
        }
    }
    
    // If statement that checks only multiple reads of the same file are ok//////////////////////////////////////////////////////////////////////////

    // 4. allocate a FREE OpenFileTable (OFT) and fill in values:
    OFT *oftp = malloc(sizeof(OFT)); // Allocates the OpenFileTable
    oftp->mode = mode; // mode = 0|1|2|3 for R|W|RW|APPEND (swapped with mode)
    oftp->shareCount = 1;
    oftp->inodeptr = mip; // point at the file's minode[]

    //   5. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:
    switch (mode) // swapped with mode
    {
    case 0:
        oftp->offset = 0; // R: offset = 0
        break;
    case 1:
        truncate(mip); // W: truncate file to 0 size ----> We need Truncate to work///////////////////////////////////////////////////////////////////
        oftp->offset = 0;
        break;
    case 2:
        oftp->offset = 0; // RW: do NOT truncate file
        break;
    case 3:
        oftp->offset = mip->INODE.i_size; // APPEND mode
        break;
    default:
        printf("invalid mode\n");
        return (-1);
    }

    // 7. find the SMALLEST index i in running PROC's fd[ ] such that fd[i] is NULL
    int i = 0;
    while (i < NFD)
    {
        if (running->fd[i] == NULL)
        {
            printf("Null found\n");
            running->fd[i] = oftp; //  Let running->fd[i] point at the OFT entry
            break;
        }
        ++i;
    }

    // 8. update INODE's time field
    // R: touch atime.
    if (mode == READ)
    {
        mip->INODE.i_atime=time(0L); // how do I update? /////////////////////////////////////////////////////////////////////////////////////////////////
    }
    else{
        // W|RW|APPEND mode : touch atime and mtime
        mip->INODE.i_atime=time(0L); // How do I update? ////////////////////////////////////////////////////////////////////////////////////////////////////
        mip->INODE.i_mtime=time(0L); // Same deal? /////////////////////////////////////////////////////////////////////////////////////////////////////////
    }
    mip->modified = 1; // mark Minode[ ] MODIFIED

    // 9. return i as the file descriptor
    printf("fd = %d\n", i);
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
    if (fd == NULL)
    { // Check if fileDiscriptor is not null
        return -1;
    }
    if (fd < 0 || fd > 15)
    { // Check if fieDiscriptor is within range
        return -1;
    }

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
int pdf()
{
//       This function displays the currently opened files as follows:

//         fd     mode    offset    INODE
//        ----    ----    ------   --------
//          0     READ    1234   [dev, ino]  
//          1     WRITE      0   [dev, ino]
//       --------------------------------------
//   to help the user know what files has been opened.
    printf("fd\tmode\toffset\tINODE [dev, ino]\n");
    for (int i = 0; i < NFD; i++)
    {
        if (running->fd[i] == NULL)
            break;
        printf("%d\t%s\t%d\t[%d, %d]\n", i, running->fd[i]->mode, running->fd[i]->offset, running->fd[i]->inodeptr->dev, running->fd[i]->inodeptr->ino);
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
//   duplicates (copy) fd[fd] into FIRST empty fd[ ] slot;
//   increment OFT's shareCount by 1;
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
//   CLOSE gd fisrt if it's already opened;
//   duplicates fd[fd] into fd[gd];
//   increment OFT's shareCount by 1;
    return 0;
}