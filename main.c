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

int INODES_PER_BLOCK = 8, ibuf[256];

#define BLKSIZE 1024

char buf[BLKSIZE];

int fd;             // opened vdisk for READ
int inodes_block;   // inodes start block number

int  nblocks, ninodes, bmap, imap, iblk, inode_start;

char gpath[128];    // token strings
char *name[32];
int n;

char *disk = "diskimage";
int dev = 0; // set this when opening disk

INODE *iget(int dev, int ino)
{
  INODE *ip;
  int i, block, offset;
  
  block = (ino-1)/8 + inodes_block; // disk block containing this inode
  offset= (ino-1)%8; // which inode in this block
  get_block(dev, block, buf);
  ip = (INODE *)buf + offset;
  return ip;
}

int get_block(int dev, int blk, char *buf)
{   
   lseek(dev, blk*BLKSIZE, SEEK_SET);
   return read(dev, buf, BLKSIZE);
}

int search(INODE *ip, char *name)
{
  // Chapter 11.4.4  int i; 
  // Exercise 6

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

      if(strcmp(temp, name)==0){
      printf("found\n");
      return dp->inode;
      }

       cp += dp->rec_len;
       dp = (DIR *)cp;
   }
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
    char *s;
    strcpy(gpath, pathname); // copy pathname to global gpath[]
    n = 0;
    s = strtok(gpath, "/"); // tokenize the path
    while (s)               // while there is a token
    {
        name[n++] = s;      // write the token to name[n]
        s = strtok(0, "/"); // tokenize again
    }
    return n;
} 

// the start of the "showblock" program
// usage: "./a.out /a/b/c/d"
// that means you will have to parse argv for the path
// see the example 'lab5.bin' program
// example usage: ./lab5.bin lost+found
int main(int argc, char *argv[])
{
   // follow the steps here: https://eecs.wsu.edu/~cs360/LAB5.html
   // open disk
   dev = open(disk, O_RDWR);

   // need to verify EXT2 FS
   // first we'll read in the super block

   // FOR BLKSIZE=1KB: SUPER block = 1

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;

    // verify the disk image is EXT2 FS
    if (sp->s_magic != 0xEF53)
    {
        printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
        exit(1);
    }


    // read in Group Descriptor 0 (in block #2)
    //FOR BLKSIZE=1KB: groupDescriptor block=2 
    get_block(dev, 2, buf);
    gp = (GD *)buf;

    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
    inode_start = gp->bg_inode_table;
    int InodesBeginBlock = inode_start;
    printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

    // mount the root
    mount_root();

    // Print contents of the root DIRectory
    //print(root);
   
    // Tokenize pathname into name[0], name[1],... name[n-1]
    tokenize(&argv[1]);

    INODE *ip = root;
     int  ino, blk, offset;

     for (int i=0; i < n; i++){
        ino = search(ip, name[i]);
        
        if (ino==0){
          printf("can't find %s\n", name[i]); 
          exit(1);
        }
  
        // Use Mailman's algorithm to Convert (dev, ino) to newINODE pointer

        INODE *newINODE = NULL;
        // set ino equal to the roots INODE number
        ino = search(root, name);
        // mailmans algorithm
        blk = (ino - 1) / INODES_PER_BLOCK + InodesBeginBlock;
        offset = (ino - 1) % INODES_PER_BLOCK;
        get_block(dev, blk, buf);
        // buf = a pointer to the buffer containing the inode table block that was read from disk
        INODE *inode_array = (INODE *) buf;
        // finds the inode corresponding to a specific offset within the inode table
        newINODE = &inode_array[offset];
    
        // ip points at newINODE of (dev, ino);
        ip = newINODE;
     }

    // Extract information from ip-> as needed:
    // Print direct block numbers;
    // direct = blocks 0 - 11
    for(int i = 0; i < 12; i++)
    {
      int block = ip->i_block[i];
      if(block){
        get_block(dev, block, ibuf);

        int i = 0;
        while(ibuf[i] && i < 256){
          printf("%d", ibuf[i]);
          i++;
        }
      }
      else
      printf("No indirect block number %d", &i);

    }

    // Print indirect block numbers;
    // indirect = block 12
      int b12 = ip->i_block[12];
      if(b12){
        get_block(dev, b12, ibuf);

        int i = 0;
        while(ibuf[i] && i < 256){
          printf("%d", ibuf[i]);
          i++;
        }
      }
      else
      printf("No indirect block number");

    //  Print double indirect block numbers, if any
    int b13 = ip->i_block[13];
      if(b13){
        get_block(dev, b12, ibuf);

        int i = 0;
        while(ibuf[i] && i < 256){
          printf("%d", ibuf[i]);
          i++;
        }
      }
      else
      printf("No double indirect block number");
}