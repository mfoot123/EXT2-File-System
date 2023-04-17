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
int  requests, hits;

MINODE* dequeue(MINODE* queue);
int enqueue(MINODE** queue, MINODE* insert);

char* oldFile, newFile;

// start up files
#include "util.c"
#include "cd_ls_pwd.c"
#include "mkdir_create.c"
#include "dalloc.c"
//#include "link_unlink.c"

int init()
{
  int i, j;

  // initialize minodes into a freeList
  for (i=0; i<NMINODE; i++){
    MINODE *mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->id = i;
    mip->next = &minode[i+1];
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

char *disk = "diskimage";

int main(int argc, char *argv[ ])
{
  char line[128];
  char buf[BLKSIZE];

  init();

  fd = dev = open(disk, O_RDWR);
  printf("dev = %d\n", dev);  // YOU should check dev value: exit if < 0

  // get super block of dev
  get_block(dev, 1, buf);
  SUPER *sp = (SUPER *)buf;  // you should check s_magic for EXT2 FS
  printf("check: superblock magic = 0x%8x OK\n", sp);

  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;
  printf("ninodes=%d  nblocks=%d\n", ninodes, nblocks);

  get_block(dev, 2, buf);
  GD *gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = inodes_start = gp->bg_inode_table;

  printf("bmap=%d  imap=%d  iblk=%d\n", bmap, imap, iblk);

  // HERE =========================================================
  MINODE *mip = freeList;         // remove minode[0] from freeList
  freeList = freeList->next;
  cacheList = mip;                // enter minode[0] in cacheList
  mip->next = 0;

  // get root INODE
  get_block(dev, iblk, buf);
  INODE *ip = (INODE *)buf + 1;   // #2 INODE
  mip->INODE = *ip;               // copy into mip->INODE

  mip->cacheCount = 1;
  mip->shareCount = 2;            // for root AND CWD
  mip->modified   = 0;

  root = mip;           // root points at #2 INODE in minode[0]

  printf("Creating P1 as running process\n");
  printf("root shareCount = %d\n", root->shareCount);
  printf("set P1's CWD to root\n");
  running->cwd = root;           // CWD = root
  // Endhere ====================================================

 /********* write code for iget()/iput() in util.c **********
          Replace code between HERE and ENDhere with
  root         = iget(dev, 2);
  running->cwd = iget(dev, 2);
 **********************************************************/
  
  while(1){
     printf("P%d running: ", running->pid);
     pathname[0] = parameter[0] = 0;

     printf("input command [cd|ls|pwd|mkdir|creat|rmdir|link|unlink|symlink |show|hits|exit] :");
     fgets(line, 128, stdin);
     line[strlen(line)-1] = 0;    // kill \n at end

     if (line[0]==0)
        continue;

     sscanf(line, "%s %s %64c", cmd, pathname, parameter);
     printf("pathname=%s parameter=%s\n", pathname, parameter);

     if (strcmp(cmd, "ls")==0)
        ls(pathname);
     else if (strcmp(cmd, "cd")==0)
        cd(pathname);
     else if (strcmp(cmd, "pwd")==0)
        pwd(running->cwd);
     else if (strcmp(cmd, "mkdir")==0)
        make_dir(pathname);
     else if (strcmp(cmd, "create")==0)
        creat_file(pathname);
     else if (strcmp(cmd, "rmdir")==0)
        rmdir(pathname);
      else if (strcmp(cmd, "link")==0)
      {
        printf("What is the name of the old file? ");
        fgets(oldFile, 128, stdin);
        printf("\n What is the name of the new file? ");
        fgets(newFile, 128, stdin);
        link(oldFile, newFile);
      }
      else if (strcmp(cmd, "unlink")==0)
        unlink(pathname);
      else if (strcmp(cmd, "unlink")==0)
      {
        printf("What is the name of the old file? ");
        fgets(oldFile, 128, stdin);
        printf("\n What is the name of the new file? ");
        fgets(newFile, 128, stdin);
        symlink(oldFile, newFile);
      }
        

     if (strcmp(cmd, "show")==0)
        show_dir(running->cwd);
     if (strcmp(cmd, "hits")==0)
        hit_ratio();
     if (strcmp(cmd, "exit")==0)
        quit();
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
  MINODE* temp = 0;
  MINODE* p;
  // while there is something in the queue
  while(p = dequeue(cacheList))
  {
    enqueue(&temp, p);
  }

  //cacheList = temp;

  printf("\nCache List:\n");
  printf("%s %s %s %s\n", "CacheCount", "Device", "Inode", "ShareCount");

  // loop through the linked list
  MINODE* mip = cacheList;
  while (mip != NULL)
  {
    // only print used minodes aka ones that have been shared
    if (mip->shareCount > 0)
    {
      printf("%d %d %d %d\n", mip->cacheCount, mip->dev, mip->ino, mip->shareCount);
    }
    mip = mip->next;
  }

  double ratio = (hits / requests) * 100;
  printf("Hit Ratio: %f\n", ratio);
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