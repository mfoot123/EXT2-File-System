#include "type.h"

/*********** globals in main.c ***********/
PROC   proc[NPROC];
PROC   *running;

MINODE minode[NMINODE];   // minodes
MINODE *freeList;         // free minodes list
MINODE *cacheList;        // cacheCount minodes list

MINODE *root;

OFT    oft[NOFT];

char gline[256];   // global line hold token strings of pathname
char *name[64];    // token string pointers
int  n;            // number of token strings

int ninodes, nblocks;
int bmap, imap, inodes_start, iblk;  // bitmap, inodes block numbers

int  fd, dev;
char cmd[16], pathname[128], parameter[128];
int  requests, hits, ifactor;

MINODE* dequeue(MINODE* queue);
int enqueue(MINODE** queue, MINODE* insert);

char* oldFile, newFile;

/**********************************************LEVEL TWO**************************************************/

OFT oft[32];

// start up files
#include "util.c"
#include "cd_ls_pwd.c"
#include "mkdir_create.c"
#include "dalloc.c"
#include "link_unlink.c"
#include "read.c"

int init()
{
  int i, j;

  // initialize minodes into a freeList
  for (i=0; i<NMINODE; i++){
    MINODE *mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->id = i;
    mip->next = &minode[i+1];
    //printf("\n\n\n\n\n%d", minode[i].ino);
    //printf("\n");
  }
  minode[NMINODE-1].next = 0;
  freeList = &minode[0];       // free minodes list

  cacheList = 0;               // cacheList = 0

  for (i=0; i<NOFT; i++)
    oft[i].shareCount = 0;     // all oft are FREE

  for (i=0; i<NPROC; i++){     // initialize procs
     PROC *p = &proc[i];
     p->uid = p->gid = i;      // uid=0 for SUPER user
     p->pid = i+1;             // pid = 1,2,..., NPROC-1

     for (j=0; j<NFD; j++)
       p->fd[j] = 0;           // open file descritors are 0
  }

  running = &proc[0];          // P1 is running
  requests = hits = 0;         // for hit_ratio of minodes cache
}

char *disk = "disk2";

int main(int argc, char *argv[ ])
{
  char line[128];
  char buf[BLKSIZE];

  init();

  fd = dev = open(disk, O_RDWR);
  printf("dev = %d\n", dev);  // YOU should check dev value: exit if < 0
  if(dev < 0) {
    puts("Unable to open disk");
    exit(1);
  }
  // get super block of dev
  get_block(dev, 1, buf);
  SUPER *sp = (SUPER *)buf;  // you should check s_magic for EXT2 FS
  inodes_per_block = BLKSIZE / sp->s_inode_size;
  if(sp->s_magic != 0xEF53)
  {
    puts("check: superblock magic check failed");
    exit(1);
  } else {
    puts("check: s_magic check ok");
    ifactor = 2;
  }

  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;
  printf("ninodes=%d  nblocks=%d  inode_size=%d\n", ninodes, nblocks, sp->s_inode_size);
  printf("inodes_per_block=%d  ifactor=%d\n", inodes_per_block, ifactor);
  get_block(dev, 2, buf);
  GD *gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = inodes_start = gp->bg_inode_table;

  printf("bmap=%d  imap=%d  iblk=%d\n", bmap, imap, iblk);
  printf("mount root\n");

  // HERE =========================================================
  MINODE *mip = freeList;         // remove minode[0] from freeList
  freeList = freeList->next;
  get_block(dev, 2, buf);

  mip = iget(dev, 2);
  // cacheList = mip;                // enter minode[0] in cacheList
  mip->next = 0;

  // get root INODE

  // INODE* ip = (INODE*)buf + 1;
  // mip->INODE = *ip;               // copy into mip->INODE

  mip->cacheCount = 1;
  mip->shareCount = 2;            // for root AND CWD
  mip->modified   = 0;
  
  root = mip;           // root points at #2 INODE in minode[0]
  printf("Creating P1 as running process\n");
  printf("root shareCount = %d\n", root->shareCount);
  printf("root inode = %d\n", root->ino);
  printf("set P1's CWD to root\n");
  running->cwd = iget(dev, 2);          // CWD = root
  // Endhere ====================================================

 /********* write code for iget()/iput() in util.c **********
          Replace code between HERE and ENDhere with
  root         = iget(dev, 2);
  running->cwd = iget(dev, 2);
 **********************************************************/
  
  while(1){
    printf("P%d running: ", running->pid);
    pathname[0] = parameter[0] = 0;
    bzero(pathname, 0);
    bzero(parameter, 0);
    printf("Level 1: [cd|ls|pwd|mkdir|creat|rmdir|link|unlink|symlink]\n");
    printf("Level 2: [open|close|lseek|pfd|read|write|cat|cp|head|tail]\n");
    printf("Misc: [show|hits|exit]\n");
    printf("input command   :");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;    // kill \n at end

    if (line[0]==0)
      continue;

    sscanf(line, "%s %s %64c", cmd, pathname, parameter);
    printf("pathname=%s parameter=%s\n", pathname, parameter);

/************************************************LEVEL 1***************************************************/
    if (strcmp(cmd, "ls")==0)
      ls(pathname);
    else if (strcmp(cmd, "cd")==0)
      cd(pathname);
    else if (strcmp(cmd, "pwd")==0)
      pwd(running->cwd);
    else if (strcmp(cmd, "mkdir")==0)
      make_dir(pathname);
    else if (strcmp(cmd, "creat")==0)
      creat_file(pathname);
    else if (strcmp(cmd, "rmdir")==0)
    {
      printf("here\n");
      rmdir(pathname);
    }
    else if (strcmp(cmd, "link")==0)
    {
      link(pathname, parameter);
    }
    else if (strcmp(cmd, "unlink")==0)
      unlink(pathname);
    else if (strcmp(cmd, "symlink")==0)
    {
      symlink(pathname, parameter);
    }
/************************************************LEVEL 2***************************************************/
    else if(strcmp(cmd, "open")==0)
    {
      open_file();
    }
    else if(strcmp(cmd, "close")==0)
    {

    }
    else if(strcmp(cmd, "lseek")==0)
    {

    }
    else if(strcmp(cmd, "pfd")==0)
    {

    }
    else if(strcmp(cmd, "read")==0)
    {
      read_file();
    }
    else if(strcmp(cmd, "write")==0)
    {

    }
    else if(strcmp(cmd, "cat")==0)
    {
      
    }
    else if(strcmp(cmd, "cp")==0)
    {

    }
    else if(strcmp(cmd, "head")==0)
    {

    }
    else if(strcmp(cmd, "tail")==0)
    {

    }

/**************************************************MISC***************************************************/
    else if (strcmp(cmd, "show")==0)
    {
      show_dir(running->cwd);
    }
    else if (strcmp(cmd, "hits")==0)
    {
      hit_ratio();
    }
    else if (strcmp(cmd, "exit")==0)
    {
      quit();
    }
    else
    {
      printf("Command not found");
    }
  }
}


int show_dir(MINODE *mip)
{
  // show contents of mip DIR: same as in LAB5
  char sbuf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  // set ip to the INODE in our MINODE
  INODE *ip = &mip->INODE;

  // ASSUME only one data block i_block[0]
  // YOU SHOULD print i_block[0] number here
  get_block(dev, ip->i_block[0], sbuf);

  dp = (DIR *)sbuf;
  cp = sbuf;
  printf("i_number  rec_len  name_len  name\n");
  while (cp < sbuf + BLKSIZE)
  {
    strncpy(temp, dp->name, dp->name_len);
    temp[dp->name_len] = 0;

    printf(" %4d      %4d     %4d      %s\n", dp->inode, dp->rec_len, dp->name_len, temp);

    cp += dp->rec_len;
    if (dp->rec_len == 0)
    {
      return 0;
    }
    dp = (DIR *)cp;
  }
}

int hit_ratio()
{
  MINODE *temp = 0;
  MINODE* p;
 
  // while there is something in the queue
  int i = cacheList->cacheCount;
  while (i != 0)
  {
    p = dequeue(cacheList);
    enqueue(&temp, p);
    i--;
  }
  cacheList = temp;

  printf("\nCacheList: CacheCounts[Device Inode]ShareCounts->Next\n");

  // loop through the linked list
  MINODE* mip = cacheList;
  printf("CacheList: ");
  while (mip)
  {
    // only print used minodes aka ones that have been shared
    printf("c%d[%d %d]s%d->", mip->cacheCount, mip->dev, mip->ino, mip->shareCount);

    mip = mip->next;
  }
  printf("null\n");
  int ratio = (100*hits) / requests;
  printf("Requests: %d  Hits: %d  HitRatio: %d%\n", requests, hits, ratio);
}

MINODE* dequeue(MINODE* queue)
{
  if (queue == NULL) {
    // empty queue, nothing to dequeue
    return NULL;
  }
  
  // save the head pointer in temp
  MINODE* temp = queue;
  // set queue equal to its next MINODE
  queue = queue->next;
  
  return temp;
}

// modifying a pointer so double star
int enqueue(MINODE** queue, MINODE* insert)
{
  while (*queue)
    queue = &(*queue)->next;
  *queue = insert;
  insert->next = 0;
}

int quit()
{
   MINODE *mip = cacheList;
   while(mip){
     if (mip->shareCount){
        mip->shareCount = 1;
        iput(mip);    // write INODE back if modified
     }
     mip = mip->next;
   }
   exit(0);
}