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

  // 1. search cacheList for minode=(dev, ino);
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->shareCount && (mip->dev==dev) && (mip->ino==ino))
    {
      //if (found){
      // inc minode's cacheCount by 1;
      mip->cacheCount++;
      // inc minode's shareCount by 1;
      mip->shareCount++;
      // return minode pointer;
      return mip;
    }
  }

  // needed (dev, ino) NOT in cacheList
  // 2. if (freeList NOT empty){
  if (!freeList){
    // remove a minode from freeList;
    mip = freeList;
    freeList = freeList->next;
    // set minode to (dev, ino), cacheCount=1 shareCount=1, modified=0;
    mip->dev = dev;
    mip->ino = ino;
    mip->cacheCount = 1;
    mip->shareCount = 1;
    mip->modified = 0;
    // load INODE of (dev, ino) from disk into minode.INODE;
    get_inode(dev, ino, &mip->INODE);
    // enter minode into cacheList;
    enter_minode(mip);
    // return minode pointer;
    return mip;
  }

  // freeList empty case:
  // 3. find a minode in cacheList with shareCount=0, cacheCount=SMALLest
  int smallest = minode[0].cacheCount;
  int smallestIndex = 0;
  for (i = 1; i < NMINODE; i++) {
    if (minode[i].cacheCount < smallest && minode[i].shareCount == 0) {
      smallest = minode[i].cacheCount;
      smallestIndex = i;
    }
  }
  mip = &minode[smallestIndex];
  // set minode to (dev, ino), shareCount=1, cacheCount=1, modified=0
  mip->dev = dev;
  mip->ino = ino;
  mip->cacheCount = 1;
  mip->shareCount = 1;
  mip->modified = 0;
  // load INODE of (dev, ino) from disk into minode.INODE;
  get_inode(dev, ino, &mip->INODE);
  // return minode pointer;
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
  block = (mip->ino - 1) / 8 + inodes_start;
  offset = (mip->ino - 1) % 8;
  // get block containing this inode
  get_block(mip->dev, block, buf);
  // ip points at INODE
  ip = (INODE *)buf + offset; 
  // copy INODE to inode in block
  *ip = mip->INODE;
  // write back to disk
  put_block(mip->dev, block, buf); 
  // mip->refCount = 0;
  midalloc(mip);
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