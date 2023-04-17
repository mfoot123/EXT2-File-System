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
    oino = path2inode(oldFile);
    if (oino == 0) {
        printf("%s does not exist\n", oldFile);
        return -1;
    }
    omip = iget(dev, oino);

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

    // check /x/y  exists and is a DIR but 'z' does not yet exist in /x/y/
    pino = path2inode(nparent);
    if (pino == 0) {
        printf("%s does not exist\n", nparent);
        iput(omip);
        return -1;
    }
    pmip = iget(dev, pino);
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
    ino = path2inode(pathname);
//    mip = iget()
    //int inode = mip->ino;

    // check if file exists
    if (ino == 0) {
        printf("File does not exist.\n");
        return;
    }

    // get the minode of the file
    mip = iget(dev, ino);

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
    pino = path2inode(parent);
    pip = iget(dev, pino);

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



// 3. ======================== HOW TO symlink ================================
//    symlink oldNAME  newNAME    e.g. symlink /a/b/c /x/y/z

//    ASSUME: oldNAME has <= 60 chars, inlcuding the ending NULL byte.

int symlink(char *oldName, char *newName) 
{
  int ino, pino;
  MINODE *mip, *pmip;
  char oldname_copy[60], path_copy[64];
  strcpy(path_copy, newName);
  strcpy(oldname_copy, oldName);
  
  // Get the device of the root inode
  dev = root->dev;

  // (1). verify oldNAME exists
  if ((ino = path2inode(oldname_copy)) == 0) {
    printf("%s does not exist\n", oldname_copy);
    return -1;
  }
  mip = iget(dev, ino);

  // and is not a directory
  if (S_ISDIR(mip->INODE.i_mode)) {
    printf("%s is a directory\n", oldname_copy);
    iput(mip);
    return -1;
  }
  iput(mip);

  // (2). creat a FILE /x/y/z
  if (my_creat(newName, 0) < 0) {
    printf("Failed to create %s\n", newName);
    return -1;
  }

  // get the inode of the new file
  ino = path2inode(newName);
  mip = iget(dev, ino);

  // (3). change /x/y/z's type to LNK (0120000)=(b1010.....)=0xA...
  mip->INODE.i_mode = 0xA000;

  // (4). write the string oldNAME into the i_block[ ], which has room for 60 chars.
  strncpy(mip->INODE.i_block, oldname_copy, 60);

  // set /x/y/z file size = number of chars in oldName
  mip->INODE.i_size = strlen(oldname_copy);

  // (5). Write the INODE of /x/y/z back to disk.
  mip->modified = 1;
  iput(mip);
  
  return 0;
}


// 4. readlink pathname: return the contents of a symLink file
int readlink(char *pathname, char *link) {

  // (1). get INODE of pathname into a minode[ ].
  int ino = path2inode(pathname);
  if (ino == 0) {
    // Path does not exist
    return -1;
  }
  MINODE *mip = iget(dev, ino);

  // (2). check INODE is a symbolic Link file.
  if (!S_ISLNK(mip->INODE.i_mode)) {
    printf("Not a symbolic link file");
    iput(mip);
    return -1;
  }

  // (3). return its string contents in INODE.i_block[ ].
  int n = mip->INODE.i_size;
  char buf[n+1];
  buf[n] = '\0';   // Add null terminator
  get_block(mip->dev, mip->INODE.i_block[0], buf);  // Read symbolic link file's block
  strcpy(link, buf);

  // Release the minode and return success
  iput(mip);
  return 0;
}
