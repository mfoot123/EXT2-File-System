/***** type.h file for CS360 Project *****/

#ifndef type_h
#define type_h

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <ext2fs/ext2_fs.h>

// define shorter TYPES, save typing efforts
typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD    *gp;
INODE *ip;

#define BLKSIZE           1024

#define NPROC                2
#define NMINODE             64
#define NFD                  8
#define NOFT                32

// read/write/rw/append modes
#define READ 0
#define WRITE 1
#define READ_WRITE 2
#define APPEND 3

int inodes_per_block;

// In-memory inodes structure
typedef struct minode{
  INODE INODE;            // disk INODE
  int   dev, ino;
  int   cacheCount;       // minode in cache count
  int   shareCount;       // number of users on this minode (refCount?)
  int   modified;         // modified while in memory (aka dirty)
  int   id;               // index ID
  struct minode *next;    // pointer to next minode

}MINODE;

// Open File Table
typedef struct oft{
  int   mode;
  int   shareCount;
  struct minode *inodeptr;
  long  offset;
} OFT;


// PROC structure
typedef struct proc{
  int   uid;            // uid = 0 or nonzero
  int   gid;            // group ID = uid
  int   pid;            // pid = 1 or 2
  struct minode *cwd;   // CWD pointer
  OFT   *fd[NFD];       // file descriptor array
} PROC;

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

/************************************************LEVEL 1***************************************************/

// util.c
int get_block(int dev, int blk, char buf[ ]);
int put_block(int dev, int blk, char buf[ ]);
int tokenize(char *pathname);
MINODE *iget(int dev, int ino);
int iput(MINODE *mip);
int search(MINODE *mip, char *name);
MINODE *path2inode(char *pathname);
int findmyname(MINODE *pip, int myino, char myname[ ]);
int findino(MINODE *mip, int *myino);

// dalloc.c
int tst_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int clr_bit(char *buf, int bit);
int incFreeInodes(int dev);
int idalloc(int dev, int ino);
int bdalloc(int dev, int blk);
//int rmdir(const char *pathname);
int rm_child(MINODE *parent, char *name);

// mkdir_creat
int enter_child(MINODE *pip, int myino, char *myname);
int my_creat(MINODE *pip, char *name);

/*********************************************************************************************************/

/************************************************LEVEL 2***************************************************/
// main.c
int show_dir(MINODE *mip);

// cat_cp.c

int my_cat(char *filename);
int my_cp(char *src, char *dest);

// read.c
int myread(int fd, char *buf, int nbytes);

// head_tail.c
int head(char *pathname);

#endif