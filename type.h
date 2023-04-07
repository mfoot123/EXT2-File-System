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

#define inodes_per_block    8

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

// Mount Table structure from textbook
typedef struct mtable{
int dev; // device number; 0 for FREE
int ninodes; // from superblock
int nblocks;
int free_blocks; // from superblock and GD
int free_inodes;
int bmap; // from group descriptor
int imap;
int iblock; // inodes start block
MINODE *mntDirPtr; // mount point DIR pointer
char devName[64]; //device name
char mntName[64]; // mount point DIR name
}MTABLE;

#endif