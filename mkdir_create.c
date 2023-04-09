#include "type.h"

// src: textbook
// tst_bit, set_bit functions
int tst_bit(char *buf, int bit){
return buf[bit/8] & (1 << (bit % 8));
}

// src: textbook
int set_bit(char *buf, int bit){
buf[bit/8] |= (1 << (bit % 8));
}

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count by 1 in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}


int ialloc(int dev)  // allocate an inode number from imap block
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       put_block(dev, imap, buf);
		
       decFreeInodes(dev);

       printf("ialloc : ino=%d\n", i+1);		
       return i+1;
    }
  }
  return 0;
}


// WRITE YOUR OWN balloc(dev) function, which returns a FREE disk block number
int balloc(int dev) {
  int i;
  char buf[BLKSIZE];

  // read block bitmap block
  get_block(dev, bmap, buf);

  for (i = 0; i < nblocks; i++) {
    if (!tst_bit(buf, i)) {
      set_bit(buf, i);
      put_block(dev, bmap, buf);
      decFreeBlocks(dev);
      printf("balloc: block=%d\n", i + 1);
      return i + 1;
    }
  }

  return 0;
}

int make_dir(char *pathname)
{
    char *parent_pathname, *child_name, *temp_pathname;
    MINODE *parent_inode;
    int parent, child;
    
    // Extract parent directory pathname and child name

    // WARNING: strtok(), dirname(), basename() destroy pathname
    // so duplicate the pathname before 
    temp_pathname = strdup(pathname);
    // parent = dirname(pathname);   parent= "/a/b" OR "a/b"
    parent_pathname = dirname(temp_pathname);
    // child  = basename(pathname);  child = "c"
    child_name = basename(pathname);
    
    // Get parent directory inode
    parent_inode = path2inode(parent_pathname);
    
    // Verify that child name does not already exist in parent directory
    child = get_inode_number(parent_inode, child_name);
    
    // Create new directory using mymkdir() function
    parent = parent_inode->ino;
    mymkdir(parent_inode, child_name);
    
    // Update parent directory metadata
    parent_inode->INODE.i_links_count++;
    parent_inode->INODE.i_atime = time(0L);
    parent_inode->INODE.i_mtime = time(0L);
    parent_inode->modified = 1;
    
    // Release parent directory inode
    iput(parent_inode);
    
    return 0;
}

