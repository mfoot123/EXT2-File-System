#include "util.h"

/**************** util.c file **************/

int get_block(int dev, int blk, char buf[ ])
{
  lseek(dev, blk*BLKSIZE, SEEK_SET);
  int n = read(fd, buf, BLKSIZE);
  return n;
}

int put_block(int dev, int blk, char buf[ ])
{
  lseek(dev, blk*BLKSIZE, SEEK_SET);
  int n = write(fd, buf, BLKSIZE);
  return n;
}

int tokenize(char *pathname)
{
  // tokenize pathname into n token strings in (global) gline[ ]
}

MINODE *iget(int dev, int ino) // return minode pointer of (dev, ino)
{
  /********** Write code to implement these ***********
  1. search cacheList for minode=(dev, ino);
  if (found){
     inc minode's cacheCount by 1;
     inc minode's shareCount by 1;
     return minode pointer;
  }

  // needed (dev, ino) NOT in cacheList
  2. if (freeList NOT empty){
        remove a minode from freeList;
        set minode to (dev, ino), cacheCount=1 shareCount=1, modified=0;

        load INODE of (dev, ino) from disk into minode.INODE;

        enter minode into cacheList;
        return minode pointer;
     }

  // freeList empty case:
  3. find a minode in cacheList with shareCount=0, cacheCount=SMALLest
     set minode to (dev, ino), shareCount=1, cacheCount=1, modified=0
     return minode pointer;

 NOTE: in order to do 3:
       it's better to order cacheList by INCREASING cacheCount,
       with smaller cacheCount in front ==> search cacheList
  ************/
}

int iput(MINODE *mip)  // release a mip
{
  /*******************
 1.  if (mip==0)                return;

     mip->shareCount--;         // one less user on this minode

     if (mip->shareCount > 0)   return;
     if (!mip->modified)        return;

 2. // last user, INODE modified: MUST write back to disk
    Use Mailman's algorithm to write minode.INODE back to disk)
    // NOTE: minode still in cacheList;
    *****************/
}

int search(MINODE *mip, char *name)
{
  /******************
  search mip->INODE data blocks for name:
  if (found) return its inode number;
  else       return 0;
  ******************/
}

MINODE *path2inode(char *pathname)
{
  /*******************
  return minode pointer of pathname;
  return 0 if pathname invalid;

  This is same as YOUR main loop in LAB5 without printing block numbers
  *******************/
}

int findmyname(MINODE *pip, int myino, char myname[ ])
{
  /****************
  pip points to parent DIR minode:
  search for myino;    // same as search(pip, name) but search by ino
  copy name string into myname[256]
  ******************/
}

int findino(MINODE *mip, int *myino)
{
  /*****************
  mip points at a DIR minode
  i_block[0] contains .  and  ..
  get myino of .
  return parent_ino of ..
  *******************/
}