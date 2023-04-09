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

int mymkdir(MINODE *pip, char *name)
{

  // 1. pip points at the parent minode[] of "/a/b", name is a string "c"

  //  2. allocate an inode and a disk block for the new directory;

  // 4. Write contents to mip->INODE to make it a DIR INODE. Mark it modified;

  // 3. MINODE *mip = iget(dev, ino); o
  //load inode into a minode[] (in order t0 write contents to the INODE in memory.
  MINODE *mip = iget(dev,ino); 
  INODE *ip = &mip->INODE;

  ip->i_mode = 0x41ED;		// OR 040755: DIR type and permissions
  ip->i_uid  = running->uid;	// Owner uid 
  ip->i_gid  = running->gid;	// Group Id
  ip->i_size = BLKSIZE;		// Size in bytes 
  ip->i_links_count = 2;	        // Links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks 
  ip->i_block[0] = bno;             // new DIR has one data block   

  //i_block[1] to i_block[14] = 0;
  for(int i = 1; i < 15; i++)
  {
    ip->i_block[i] = 0;
  }
 
  mip->modified = 1;            // mark minode MODIFIED
  // 5. iput(mip); which writes the new INODE out to disk.
  iput(mip);

  // 6. Write . and .. entries to a buf[ ] of BLKSIZE
  // Then, write buf[ ] to the disk block bno;

  // 7. Finally, enter name ENTRY into parent's directory by 
  // enter_child(pip, ino, name);

  // 8. int enter_child(MINODE *pip, int myino, char *myname)
  // (1). NEED_LEN = 4*[ (8 + strlen(myname) + 3)/4 ]; // a multiple of 4
  // (2). For each data block of parent DIR do { // assume: only 12 direct blocks
    // if (i_block[i]==0) BREAK;
    // get parent's data block into a buf[ ]
    // Each DIR entry has rec_len, name_len. Each entry's ideal length is
    // IDEAL_LEN = 4*[ (8 + name_len + 3)/4 ]     // multiple of 4
  // (3). Step through each DIR entry dp in parent data block:
  // compute REMAIN = dp->rec_len - IDEAL_LEN;
  // if (REMAIN >= NEED_LEN){      // found enough space for new entry
  // dp->rec_len = IDEAL_LEN;   // trim dp's rec_len to its IDEAL_LEN
  // enter new_entry as [myino, REMIN, strlen(myname), myname]
  
}

