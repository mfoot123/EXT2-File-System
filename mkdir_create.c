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
    MINODE *pip;
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
    pip = path2inode(parent_pathname);
    if (!pip || !S_ISDIR(pip->INODE.i_mode)) {
      printf("Error: Parent directory does not exist or is not a directory.\n");
      return -1;
    }
    
    // Verify that child name does not already exist in parent directory
    child = get_inode_number(pip, child_name);
    if (child) {
      printf("Error: A file or directory with the same name already exists in the parent directory.\n");
      iput(pip);
      return -1;
    }
    
    // Create new directory using mymkdir() function
    parent = pip->ino;
    mymkdir(pip, child_name);
    
    // Update parent directory metadata
    pip->INODE.i_links_count++;
    pip->INODE.i_atime = time(0L);
    pip->modified = 1;
    
    // Release parent directory inode
    iput(pip);
    
    return 0;
}

