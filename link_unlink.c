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

int link(char* oldFile, char* newFile) 
{

    // variables
    int oino, pino;
    char oparent[64], ochild[64], nparent[64], nchild[64];
    MINODE *omip, *pmip;
    INODE *ip;

    // (1). get the INODE of /a/b/c (oldFile) into memory
    omip = path2inode(oldFile);
    oino = omip->ino;
    if (oino == 0) {
        printf("%s does not exist\n", oldFile);
        return -1;
    }
    //omip = iget(dev, oino);

    // (2). check /a/b/c (oldFile) is NOT a DIRectory (link to DIR is NOT allowed)
    if (S_ISDIR(omip->INODE.i_mode)) {
        printf("%s is a directory, cannot link\n", oldFile);
        iput(omip);
        return -1;
    }

    // (3). break up paramters into parent (/x/y), child (z)
    strcpy(oparent, dirname(oldFile));
    strcpy(ochild, basename(oldFile));
    strcpy(nparent, dirname(newFile));
    strcpy(nchild, basename(newFile));
    printf("%s", nparent);

    // check /x/y  exists and is a DIR but 'z' does not yet exist in /x/y/
    pmip = path2inode(nparent);
    pino = pmip->ino;
    if (pino == 0) {
        printf("%s does not exist\n", nparent);
        iput(omip);
        return -1;
    }
    //pmip = iget(dev, pino);
    printf("pino:%d pmip:%d", pino, pmip->ino);
    if (!S_ISDIR(pmip->INODE.i_mode)) {
        printf("%s is not a directory, cannot link\n", nparent);
        iput(omip);
        iput(pmip);
        return -1;
    }
    if (search(pmip, nchild)) {
        printf("%s already exists in %s\n", nchild, nparent);
        iput(omip);
        iput(pmip);
        return -1;
    }

    // (4). Add an entry [ino rec_len name_len z] to the data block of /x/y/
    // Add an entry for nchild in nparent with the same inode number as omip
    enter_child(pmip, oino, nchild);

    // (5). increment the i_links_count of INODE by 1
    omip->INODE.i_links_count++;
    omip->modified = 1;

    // (6). write back to disk   
    iput(omip);
    iput(pmip);

    return 0;
}


// ===========================================================================
// 2.                     HOW TO unlink
//      unlink pathname
void unlink(char *pathname)
{
    int ino, pino, i;
    MINODE *mip, *pip;
    char parent[256], child[256], temp[256];

    // get the ino of the file to be unlinked
    mip = path2inode(pathname);
    ino = mip->ino;
//    mip = iget()
    //int inode = mip->ino;

    // check if file exists
    if (ino == 0) {
        printf("File does not exist.\n");
        return;
    }

    // get the minode of the file
    //mip = iget(dev, ino);

    // check if file is a directory
    if (S_ISDIR(mip->INODE.i_mode)) {
        printf("Cannot unlink a directory.\n");
        iput(mip);
        return;
    }

    // get the name of the parent directory and the basename of the file
    strcpy(temp, pathname);
    strcpy(parent, dirname(temp));
    strcpy(temp, pathname);
    strcpy(child, basename(temp));

    // get the minode of the parent directory
    pip = path2inode(parent);
    pino = pip->ino;
    //pip = iget(dev, pino);

    // remove the file from the parent directory
    rm_child(pip, child);

    // update the parent directory's time fields
    pip->INODE.i_mtime = time(0L);
    pip->INODE.i_ctime = time(0L);
    pip->modified = 1;

    // decrement the link count of the file
    mip->INODE.i_links_count--;

    // if link count is still greater than 0, just write the minode back to disk and return
    if (mip->INODE.i_links_count > 0) {
        mip->modified = 1;
        iput(mip);
        iput(pip);
        return;
    }

    // deallocate the file's blocks and inode
    truncate(mip);

    // deallocate the inode
    mip->INODE.i_mode = 0;
    mip->modified = 1;
    iput(mip);

    // update the parent directory's size and link count
    pip->INODE.i_links_count--;
    //pip->INODE.i_size -= DIR_ENTRY_SIZE;
    pip->modified = 1;

    // write the parent directory's minode back to disk and return
    iput(pip);
}

void truncate(MINODE *mip) {
  // deallocates all the data blocks of the given inode

  int i, j, k;
  char buf[BLKSIZE];
  int *temp;

  // deallocate direct blocks
  for (i = 0; i < 12; i++) {
    if (mip->INODE.i_block[i]) {
      bdalloc(mip->dev, mip->INODE.i_block[i]);
      mip->INODE.i_block[i] = 0;
    }
  }

  // deallocate indirect block
  if (mip->INODE.i_block[12]) {
    get_block(mip->dev, mip->INODE.i_block[12], buf);
    temp = (int*)buf;
    for (i = 0; i < 256 && *temp; i++) {
      bdalloc(mip->dev, *temp);
      temp++;
    }
    bdalloc(mip->dev, mip->INODE.i_block[12]);
    mip->INODE.i_block[12] = 0;
  }

  // deallocate double indirect block
  if (mip->INODE.i_block[13]) {
    get_block(mip->dev, mip->INODE.i_block[13], buf);
    temp = (int*)buf;
    for (i = 0; i < 256 && *temp; i++) {
      get_block(mip->dev, *temp, buf);
      for (j = 0; j < 256 && *temp; j++) {
        bdalloc(mip->dev, *temp);
        temp++;
      }
    }
    bdalloc(mip->dev, mip->INODE.i_block[13]);
    mip->INODE.i_block[13] = 0;
  }

  mip->INODE.i_size = 0;
  mip->INODE.i_blocks = 0;
  mip->modified = 1;
  iput(mip);
}


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
    mip->INODE.i_mode = 0xA1FF; // A1FF sets link perm bits correctly (rwx for all users)
    mip->modified = 1;

    // (4). write the string oldNAME into the i_block[ ], which has room for 60 chars.
    // i_block[] + 24 after = 84 total for old
    strncpy(mip->INODE.i_block, oldNAME, 84);

    // set /x/y/z file size = number of chars in oldName
    mip->INODE.i_size = strlen(oldNAME) + 1; // +1 for '\0'

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