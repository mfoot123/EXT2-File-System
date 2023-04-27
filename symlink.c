#include "type.h"

// 3. ======================== HOW TO symlink ================================
//    symlink oldNAME  newNAME    e.g. symlink /a/b/c /x/y/z

//    ASSUME: oldNAME has <= 60 chars, inlcuding the ending NULL byte.

// SOURCE: https://github.com/Eastonco/CS360/blob/master/lab6/src/cmd/symlink.c
// NOTE: Code was not directly copy and pasted
//       It had to be modified to accomodate new requirements

int symlink(char *oldNAME, char *newNAME) {
    MINODE *mip;

    // set dev correctly for getting old inode
    if (oldNAME[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    // (1). verify oldNAME exists (either a DIR or a REG file)
    mip = path2inode(oldNAME);
    int old_ino = mip->ino;
    if (old_ino == 0) {
        printf("%s does not exist\n", oldNAME);
        return -1;
    }
    if (!S_ISDIR(mip->INODE.i_mode) && !S_ISREG(mip->INODE.i_mode)) {
        printf("%s is neither a directory nor a regular file\n", oldNAME);
        iput(mip);
        return -1;
    }

    // set dev correctly for getting new inode
    if (newNAME[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    // (2). creat a FILE /x/y/z
    creat_file(newNAME);

    // (3). change /x/y/z's type to LNK (0120000)=(b1010.....)=0xA...
    mip = path2inode(newNAME);
    int new_ino = mip->ino;
    if (new_ino == 0) {
        printf("%s does not exist\n", newNAME);
        return -1;
    }
    //mip = iget(dev, new_ino);
    mip->INODE.i_mode &= 0x0FFF; // 0x0FFF clears first four bits of imode
    mip->INODE.i_mode |= 0xA000;
    mip->modified = 1;

    // (4). write the string oldNAME into the i_block[ ], which has room for 60 chars.
    // i_block[] + 24 after = 84 total for old
    strncpy(mip->INODE.i_block, oldNAME, 84);

    // set /x/y/z file size = number of chars in oldName
    mip->INODE.i_size = strlen(oldNAME);

    // (5). write the INODE of /x/y/z back to disk.
    iput(mip);
}




// 4. readlink pathname: return the contents of a symLink file
char* readlink(char *pathname)
{
  MINODE *mip;
  char *link_str;
  
  // (1). get INODE of pathname into a minode[ ].
  mip = path2inode(pathname);
  if (!mip) {
    printf("Failed to get inode of %s\n", pathname);
    return NULL;
  }
  
  // (2). check INODE is a symbolic Link file.
  if (!S_ISLNK(mip->INODE.i_mode)) {
    printf("%s is not a symbolic link file\n", pathname);
    iput(mip);
    return NULL;
  }
  
  // (3). return its string contents in INODE.i_block[ ].
  link_str = (char*)malloc(mip->INODE.i_size+1); // add space for NULL byte
  strncpy(link_str, mip->INODE.i_block, mip->INODE.i_size);
  link_str[mip->INODE.i_size] = '\0'; // add NULL byte
  iput(mip);
  
  return link_str;
}