#include "type.h"

/*
* Open File
*
*
*
*
*
*
*/ 
int open_file()
{
//       1. ASSUME open pathname mode   # mode = 0|1|2|3 for R|W|RW|APPEND

//   2. get pathname's minode pointer:

//          MINODE *mip = path2inode(pathname);

//   3. if (!mip){ // pathname does not exist:
//         if (mode==0)    // READ: file must exist
// 	    return -1;
		    
//         // mode=R|RW|APPEND: creat file first
//         creat_file();  // make sure YOUR creat_file() use pathname 

// 	mip = pathn2inode(pathname);
// 	// print mip's [dev, ino]
//      }

//   4. check mip->INODE.i_mode to verify it's a REGULAR file
      
//      Check whether the file is ALREADY opened with INCOMPATIBLE mode:
//      If it's already opened for W, RW, APPEND : reject.
//      that is, only multiple READs of the SAME file are OK

//   4. allocate a FREE OpenFileTable (OFT) and fill in values: 
 
//       oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 
//       oftp->shareCount = 1;
//       oftp->minodeptr = mip;  // point at the file's minode[]

//   5. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:

//       switch(mode){
//          case 0 : oftp->offset = 0;     // R: offset = 0
//                   break;
//          case 1 : truncate(mip);        // W: truncate file to 0 size
//                   oftp->offset = 0;
//                   break;
//          case 2 : oftp->offset = 0;     // RW: do NOT truncate file
//                   break;
//          case 3 : oftp->offset =  mip->INODE.i_size;  // APPEND mode
//                   break;
//          default: printf("invalid mode\n");
//                   return(-1);
//       }

//    7. find the SMALLEST index i in running PROC's fd[ ] such that fd[i] is NULL
//       Let running->fd[i] point at the OFT entry

//    8. update INODE's time field
//          for R: touch atime. 
//          for W|RW|APPEND mode : touch atime and mtime
//       mark Minode[ ] MODIFIED

//    9. return i as the file descriptor
    return 0;
}

/*
 * Truncate
 *
 *
 *
 *
 *
 *
 */
int truncate(MINODE *mip)
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

/*
 * CloseFile
 *
 *
 *
 *
 *
 *
 */
int close_file(int fd)
{
//       1. verify fd is within range.

//   2. verify running->fd[fd] is pointing at a OFT entry

//   3. The following code segments should be obvious:
		    
//      oftp = running->fd[fd];
//      running->fd[fd] = 0;

//      oftp->shareCount--;
//      if (oftp->shareCount > 0)
//         return 0;

//      // last user of this OFT entry ==> dispose of its minode
//      mip = oftp->inodeptr;
//      iput(mip);
    return 0;
}

/*
 * LSeek
 *
 *
 *
 *
 *
 *
 */
int lseek(int fd, int position)
{
//       From fd, find the OFT entry. 

//   change OFT entry's offset to position but make sure NOT to over run either end
//   of the file.

//   return originalPosition
    return 0;
}

/*
 * PDF
 *
 *
 *
 *
 *
 *
 */
int pdf()
{
//       This function displays the currently opened files as follows:

//         fd     mode    offset    INODE
//        ----    ----    ------   --------
//          0     READ    1234   [dev, ino]  
//          1     WRITE      0   [dev, ino]
//       --------------------------------------
//   to help the user know what files has been opened.
    return 0;
}

/*
 * File Descriptor
 *
 *
 *
 *
 *
 *
 */
int dup(int fd){
//       verify fd is an opened descriptor;
//   duplicates (copy) fd[fd] into FIRST empty fd[ ] slot;
//   increment OFT's shareCount by 1;
    return 0;
}

/*
 * File Descriptor 2
 *
 *
 *
 *
 *
 *
 */
int dup2(int fd, int gd)
{
//       CLOSE gd fisrt if it's already opened;
//   duplicates fd[fd] into fd[gd];
//   increment OFT's shareCount by 1;
    return 0;
}