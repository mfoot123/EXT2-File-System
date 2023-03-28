#include "type.h"

/********** globals **************/
PROC   proc[NPROC];
PROC   *running;

MINODE minode[NMINODE];   // in memory INODES
MINODE *freeList;         // free minodes list
MINODE *cacheList;        // cached minodes list

MINODE *root;             // root minode pointer

OFT    oft[NOFT];         // for level-2 only

char gline[256];          // global line hold token strings of pathname
char *name[64];           // token string pointers
int  n;                   // number of token strings

int ninodes, nblocks;     // ninodes, nblocks from SUPER block
int bmap, imap, inodes_start, iblk;  // bitmap, inodes block numbers

int  fd, dev;
char cmd[16], pathname[128], parameter[128];
int  requests, hits;

// start up files
#include "util.c"
#include "cd_ls_pwd.c"

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

  // get root INODE
  get_block(dev, iblk, buf);
  INODE *ip = (INODE *)buf + 1;   // #2 INODE
  mip->INODE = *ip;               // copy into mip->INODE

  mip->cacheCount = 1;
  mip->shareCount = 2;            // for root AND CWD
  mip->modified   = 0;

  root = mip;           // root points at #2 INODE in minode[0]

  printf("set P1's CWD to root\n");
  running->cwd = root;           // CWD = root
  // Endhere ====================================================

 /********* write code for iget()/iput() in util.c **********
          Replace code between HERE and ENDhere with

  root         = iget(dev, 2);
  running->cwd = iget(dev, 2);
 **********************************************************/

  while(1){
     printf("P%d running\n", running->pid);
     pathname[0] = parameter[0] = 0;

     printf("enter command [cd|ls|pwd|exit] : ");
     fgets(line, 128, stdin);
     line[strlen(line)-1] = 0;    // kill \n at end

     if (line[0]==0)
        continue;

     sscanf(line, "%s %s %64c", cmd, pathname, parameter);
     printf("pathname=%s parameter=%s\n", pathname, parameter);

     if (strcmp(cmd, "ls")==0)
        ls();
     if (strcmp(cmd, "cd")==0)
        cd();
     if (strcmp(cmd, "pwd")==0)
        pwd();


     if (strcmp(cmd, "show")==0)
        show_dir();
     if (strcmp(cmd, "hits")==0)
        hit_ratio();
     if (strcmp(cmd, "exit")==0)
        quit();
  }
}


int show_dir(MINODE *mip)
{
  // show contents of mip DIR: same as in LAB5
}

int hit_ratio()
{
  // print cacheList;
  // compute and print hit_ratio
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