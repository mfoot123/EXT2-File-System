#include "type.h"

/*********** globals in main.c ***********/
extern PROC   proc[NPROC];
extern PROC   *running;

extern MINODE minode[NMINODE];   // minodes
extern MINODE *freeList;         // free minodes list
extern MINODE *cacheList;        // cacheCount minodes list

extern MINODE *root;

extern OFT    oft[NOFT];

extern char gline[256];   // global line hold token strings of pathname
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings

extern int ninodes, nblocks;
extern int bmap, imap, inodes_start, iblk;  // bitmap, inodes block numbers

extern int  fd, dev;
extern char cmd[16], pathname[128], parameter[128];
extern int  requests, hits;

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
    char *s;
    strcpy(gline, pathname); // copy pathname to global gpath[]
    n = 0;
    s = strtok(gline, "/"); // tokenize the path
    while (s)               // while there is a token
    {
        name[n++] = s;      // write the token to name[n]
        s = strtok(0, "/"); // tokenize again
    }
    return n;
}

MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;

  requests++;

  mip = cacheList;
  while (mip != NULL) {
    if (mip->shareCount && (mip->dev == dev) && (mip->ino == ino)) {
      // If found: 
      //increment minode's cacheCount and shareCount
      mip->cacheCount++;
      mip->shareCount++;
      // update our hits
      hits++;
      // return minode pointer
      return mip;
    }
    // traverse to next MINODE in the list
    mip = mip->next;
  }

  // Needed (dev, ino) NOT in cacheList
  if (!freeList) {
    // If freeList is empty, allocate a new MINODE
    mip = (MINODE *)malloc(sizeof(MINODE));
  } else {
    // Otherwise, remove a minode from freeList
    mip = freeList;
    freeList = freeList->next;
  }

  // Set minode to (dev, ino), cacheCount=1 shareCount=1, and modified=0
  mip->dev = dev;
  mip->ino = ino;
  mip->cacheCount = 1;
  mip->shareCount = 1;
  mip->modified = 0;

  // Add the new minode to the end of the cacheList
  int j;
  for (j = 0; j < NMINODE; j++) {
    if (!minode[j].dev) {
      minode[j] = *mip;
      break;
    }
  }

  // Return minode pointer
  return mip;
}

int iput(MINODE *mip)  // release a mip
{
  INODE *ip;
  int i, block, offset;
  char buf[BLKSIZE];
  // 1.  if (mip==0)                return;
  if (mip==0) return;
  // one less user on this minode 
  mip->shareCount--; // dec refCount by 1
  if (mip->shareCount > 0) return; // still has user
  if (!mip->modified) return; // no need to write back

  //  2. last user, INODE modified: MUST write back to disk
  // Use Mailman's algorithm to write minode.INODE back to disk)
  block = (mip->ino - 1) / inodes_per_block + inodes_start;
  offset = (mip->ino - 1) % inodes_per_block;
  // get block containing this inode
  get_block(mip->dev, block, buf);
  // ip points at INODE
  ip = (INODE *)buf + offset; 
  // copy INODE to inode in block
  *ip = mip->INODE;
  // write back to disk
  put_block(mip->dev, block, buf); 
  // mip->refCount = 0;
  mip->shareCount = 0;
}

int search(MINODE *mip, char *name)
{
  /******************
  search mip->INODE data blocks for name:
  if (found) return its inode number;
  else       return 0;
  ******************/
  int i;
  char *cp, temp[256], sbuf[BLKSIZE];
  DIR *dp;
  for (i=0; i<12; i++){ // search DIR direct blocks only

    if (mip->INODE.i_block[i] == 0)
    return 0;

    // search mip->INODE data blocks for name:
    get_block(mip->dev, mip->INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    while (cp < sbuf + BLKSIZE){
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      printf("%8d%8d%8u %s\n",
      dp->inode, dp->rec_len, dp->name_len, temp);
      if (strcmp(name, temp)==0){
      printf("found %s : inumber = %d\n", name, dp->inode);
      // if (found) return its inode number;
      return dp->inode;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  return 0;
}

MINODE *path2inode(char *pathname)
{
  MINODE *mip = root;
  char buf[BLKSIZE];
  int  ino, blk, offset;
  /*******************
  return minode pointer of pathname;
  return 0 if pathname invalid;

  This is same as YOUR main loop in LAB5 without printing block numbers
  *******************/
  if(strcmp(pathname, ".") == 0 || pathname == NULL)
  {
    //return cwd
    return running->cwd;
  }
  else if(strcmp(pathname, "..") == 0)
  {
    // return parent inode
  }

  // determine absolute or relative

  // tokenize the path

  // call search to get to c

  // mailman’s algorithm

  // call iget(dev, ino)

}

int findmyname(MINODE *pip, int myino, char myname[ ])
{
  /****************
  pip points to parent DIR minode:
  search for myino;    // same as search(pip, name) but search by ino
  copy name string into myname[256]
  ******************/
    char *cp;
    INODE *ip;
    int i, blk, offset;
    DIR *dp;
    char buf[BLKSIZE];

    // Get the inode of the parent directory
    ip = &pip->INODE;

    // Calculate the block and offset of the directory entry
    blk = (myino - 1) / (BLKSIZE / sizeof(DIR)) + ip->i_block[0];
    offset = (myino - 1) % (BLKSIZE / sizeof(DIR));

    // Read the block that contains the directory entry
    get_block(pip->dev, blk, buf);

    // Get a pointer to the directory entry
    dp = (DIR *) buf + offset;

    // Copy the name of the inode into myname 
    cp = dp->name;
    strncpy(myname, cp, strlen(cp));
    myname[strlen(cp)] = '\0';

    return 0;
}

int findino(MINODE *mip, int *myino)
{
  DIR *dp;
  /*****************
  mip points at a DIR minode
  i_block[0] contains .  and  ..
  get myino of .
  return parent_ino of ..
  *******************/
 // Check if mip points at a DIR minode
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("Error: inode is not a directory\n");
        return -1;
    }

    // Get the inode number of the current directory (.)
    *myino = mip->ino;

    // Get the parent inode number (..)
    if (mip->ino == root) {
        // Special case: the root directory
        return 0;
    } else {
        // Get the parent directory from the current directory's inode
        int parent_block = mip->INODE.i_block[0];
        char parent_buf[BLKSIZE];
        get_block(mip->dev, parent_block, parent_buf);

        // The parent directory is stored in the second entry of the block
        // (the first entry is for ".", the second is for "..")
        char *cp = parent_buf;
        dp = (DIR *)cp;
        cp += dp->rec_len;
        dp = (DIR *)cp;
        *myino = dp->inode;

        return 0;
    }
}