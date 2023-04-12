#include "type.h"

/*
1. link oldFileName newFileName (link pathname parameter)

   creates a file newFileName which has the SAME inode (number) as that of
   oldFileName.


   Example: link     /a/b/c                      /x/y/z ==>
                     /a/b/   data block          /x/y    data block
                 ------------------------        -------------------------
                .. .|ino rlen nlen c|...        ....|ino rlen nlen z| ....
                ------|-----------------        ------|------------------
                      |                               |
                    INODE <----------------------------

                i_links_count = 1  <== INCrement i_links_count to 2


*/
int link(char* oldFile, char* newFile){
/*
    (1). get the INODE of /a/b/c into memory: mip->minode[ ]
                                               INODE of /a/b/c
                                               dev,ino
                                               .......
    mip = path2inode(pathname);
*/
    MINODE *omip;
    omip = path2inode(oldFile);
    int oino = mip->ino;
    int odev = mip->dev;

    // (2). check /a/b/c is NOT a DIRectory (link to DIR is NOT allowed).
    // mip = iget(dev, ino); ???????????????????????????

    // Verify mip->INODE is a DIR // return error if DIR
    if (S_ISDIR(omip->INODE.i_mode))
    {
        printf("%s: Is a directory\n", oldFile);
        // release the mip
        iput(omip);
        return -1;
    }

    MINODE *newOMip;
    newOMip = path2inode(newFile);
    // Checks to see if file exists already
    if (newOMip->ino != 0)
    {
        printf("%s: Exists\n", oldFile);
        return -1; // Throws off an error
    }
    
    // (3). break up paramter into parent (/x/y), child (z)
    //      check /x/y  exists and is a DIR but 'z' does not yet exist in /x/y/
    char *parent, child;
    int pino;
    MINODE *pmip;
    strcpy(parent, dirname(newFile));
    strcpy(child, basename(newFile));
    pmip = path2inode(parent);
    pino = pmip->ino;

    // (4). Add an entry [ino rec_len name_len z] to the data block of /x/y/
    //      This creates /x/y/z, which has the SAME ino as that of /a/b/c
    // (NOTE: both /a/b/c and /x/y/z must be on the SAME device;
    //        link can not be across different devices).
    enter_name(pmip, oino, child);

    // (5). increment the i_links_count of INODE by 1
    omip->INODE.i_links_count++;
    omip->dirty = 1;

    // (6). write INODE back to disk
    iput(pmip);
    iput(newOMip);
}


// ===========================================================================
// 2.                     HOW TO unlink
//      unlink pathname
int unlink(char *linkedFile)
{
    // (1). get pathname's INODE into memory
    MINODE *mip = iget(linkedFile);
    int ino = mip->ino;

    // check if its a file or directory
    if (S_ISDIR(mip->inode.i_mode))
    {
        printf("%s: Is a directory", linkedFile);
        return -1;
    }
    else if (S_ISREG(mip->inode.i_mode))
    {
        printf("%s: is a file", linkedFile);
        char *parent = dirname(linkedFile);
        char *child = basename(linkedFile);

        MINODE *pmip = iget(linkedFile);
        int pino = pmip->ino;
        rm_child(pmip, ino, child);
        pmip->dirty = 1;
        iput(pmip);
    }
    
    // (2). verify it's a FILE (REG or LNK), can not be a DIR;

    // (3). decrement INODE's i_links_count by 1;

    // (4). if i_links_count == 0 ==> rm pathname by

    //         deallocate its data blocks by:

    //      Write a truncate(INODE) function, which deallocates ALL the data blocks
    //      of INODE. This is similar to printing the data blocks of INODE.

    //         deallocate its INODE;

    // (5). Remove childName = basename(pathname) from the parent directory by

    //         rm_child(parentInodePtr, childName)

    //      which is the SAME as that in rmdir or unlink file operations.
    return 0;
}

// 3. ======================== HOW TO symlink ================================
//    symlink oldNAME  newNAME    e.g. symlink /a/b/c /x/y/z

//    ASSUME: oldNAME has <= 60 chars, inlcuding the ending NULL byte.

int symlink(char *oldName, char *newName)
{
    // (1). verify oldNAME exists (either a DIR or a REG file)
    // (2). creat a FILE /x/y/z
    // (3). change /x/y/z's type to LNK (0120000)=(b1010.....)=0xA...
    // (4). write the string oldNAME into the i_block[ ], which has room for 60 chars.

    //      set /x/y/z file size = number of chars in oldName

    // (5). write the INODE of /x/y/z back to disk.
    return 0;
}

// 4. readlink pathname: return the contents of a symLink file
int readlink(char* newName){
// (1). get INODE of pathname into a minode[ ].
// (2). check INODE is a symbolic Link file.
// (3). return its string contents in INODE.i_block[ ].
    return 0;
}
