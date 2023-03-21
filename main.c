/************** lab5base.c file ******************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp; 

// declare root
INODE *root;

#define BLKSIZE 1024

int fd;             // opened vdisk for READ
int inodes_block;   // inodes start block number

int  nblocks, ninodes, bmap, imap, iblk, inode_start;

char gpath[128];    // token strings
char *name[32];
int n;

char *disk = "diskimage";
int dev = 0; // set this when opening disk

int get_block(int dev, int blk, char *buf)
{   
   lseek(dev, blk*BLKSIZE, SEEK_SET);
   return read(dev, buf, BLKSIZE);
}

int search(INODE *ip, char *name)
{
  // Chapter 11.4.4  int i; 
  // Exercise 6
}

int show_dir(INODE *ip)
{
   char sbuf[BLKSIZE], temp[256];
   DIR *dp;
   char *cp;
 
   // ASSUME only one data block i_block[0]
   // YOU SHOULD print i_block[0] number here
   get_block(dev, ip->i_block[0], sbuf);

   dp = (DIR *)sbuf;
   cp = sbuf;
 
   while(cp < sbuf + BLKSIZE){
       strncpy(temp, dp->name, dp->name_len);
       temp[dp->name_len] = 0;
      
       printf("%4d %4d %4d %s\n", dp->inode, dp->rec_len, dp->name_len, temp);

       cp += dp->rec_len;
       dp = (DIR *)cp;
   }
}

int mount_root()
{
  // Let INODE *root point at root INODE (ino=2) in memory:
  root = iget(dev, 2);
}

/*************************************************************************/
int tokenize(char *pathname)
{
  // you did this many times
} 

// the start of the "showblock" program
// usage: "./a.out /a/b/c/d"
// that means you will have to parse argv for the path
// see the example 'lab5.bin' program
// example usage: ./lab5.bin lost+found
int main(int argc, char *argv[])
{
   // follow the steps here: https://eecs.wsu.edu/~cs360/LAB5.html
   char buf[BLKSIZE];

   // open disk
   dev = open(disk, O_RDWR);

   // need to verify EXT2 FS
   // first well read in the super block

   // FOR BLKSIZE=1KB: SUPER block = 1

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;

    // verify the disk image is EXT2 FS



    // read in Group Descriptor 0 (in block #2)
    get_block(dev, 2, buf);
    gp = (GD *)buf;

    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
    inode_start = gp->bg_inode_table;
    printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

    // mount the root
    mount_root();

    // Print contents of the root DIRectory
    
   
}