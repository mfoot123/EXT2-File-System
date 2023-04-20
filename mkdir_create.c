#include "type.h"

/****************************************************HELPER FUNCTIONS******************************************************/


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
      decFreeInodes(dev);
      printf("balloc: block=%d\n", i + 1);
      return i + 1;
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/******************************************************MAKE DIR************************************************************/

int mymkdir(MINODE *pip, char *name)
{
  int ino;
  char buf[BLKSIZE], temp[256];
  DIR* dp;
  char *parent_path;

  // 1. pip points at the parent minode[] of "/a/b", name is a string "c"

  //char *path = "/a/b";
  //char *parent_path = dirname(path); // parent_path is now "/a"

  // Find the inode of the parent directory "/a"
  /*MINODE *parent_mip = path2inode(parent_path);
  if (parent_mip == NULL) {
      printf("Error: directory %s does not exist\n", parent_path);
      return -1;
  }
  */

  // Get the MINODE structure for the parent inode
  //pip = iget(root->dev, parent_mip->ino);

  //  2. allocate an inode and a disk block for the new directory;
  ino = ialloc(dev);   // Allocate a new inode on the device
  int blk = balloc(dev);   // Allocate a new block on the device

  // 3. MINODE *mip = iget(dev, ino); o
  //load inode into a minode[] (in order t0 write contents to the INODE in memory.
  MINODE *mip = iget(dev,ino); 
  INODE *ip = &mip->INODE;

  // 4. Write contents to mip->INODE to make it a DIR INODE. Mark it modified;

  ip->i_mode = 0x41ED;		// OR 040755: DIR type and permissions
  ip->i_uid  = running->uid;	// Owner uid 
  ip->i_gid  = running->gid;	// Group Id
  ip->i_size = BLKSIZE;		// Size in bytes 
  ip->i_links_count = 2;	        // Links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks 
  // Set the first block of the new directory to the block we just allocated
  ip->i_block[0] = blk;             // new DIR has one data block   

  //i_block[1] to i_block[14] = 0;
  for(int i = 1; i < 15; i++)
  {
    ip->i_block[i] = 0;
  }
 
  mip->modified = 1;            // mark minode MODIFIED
  // 5. iput(mip); which writes the new INODE out to disk.
  iput(mip);

  // 6. Write . and .. entries to a buf[ ] of BLKSIZE
  dp = (DIR *)buf;

  // Create the "." directory entry and write it to the buffer
  dp->inode = ino;
  dp->name_len = 1;
  dp->name[0] = '.';
  dp->rec_len = 12;
  dp = (char *)dp + dp->rec_len;

  // Create the ".." directory entry and write it to the buffer
  dp->inode = pip->ino;
  dp->name_len = 2;
  dp->name[0] = dp->name[1] = '.';
  dp->rec_len = BLKSIZE - 12;

  // Write the buffer to the disk block we allocated earlier
  put_block(dev, blk, buf);



  // 7. Finally, enter name ENTRY into parent's directory by 
  // enter_child(pip, ino, name);
  enter_child(pip, ino, name);
  
}

int enter_child(MINODE *pip, int myino, char *myname)
{
    int i;
    char *cp;
    DIR *dp;
    int ideal_len, remain;
    char buf[BLKSIZE];

    // (1). NEED_LEN = 4*[ (8 + strlen(myname) + 3)/4 ]; // a multiple of 4
    int need_len = 4 * ((8 + strlen(myname) + 3) / 4);
    
    // (2). For each data block of parent DIR do { 
    // assume: only 12 direct blocks
    for (i = 0; i < 12; i++) {
        // if (i_block[i]==0) BREAK;
        // no dir block here
        if (pip->INODE.i_block[i] == 0) // if no more data blocks, break
            break;
        
        // get parent's data block into a buf[ ]
        get_block(pip->dev, pip->INODE.i_block[i], buf);
        cp = buf;
        dp = (DIR *) buf;
        
        // (3). Step through each DIR entry dp in parent data block:
        while (cp < buf + BLKSIZE) {
            // Compute ideal length and remaining space in DIR entry

            // Each DIR entry has rec_len, name_len. Each entry's ideal length is
            // IDEAL_LEN = 4*[ (8 + name_len + 3)/4 ]     // multiple of 4

            ideal_len = 4 * ((8 + dp->name_len + 3) / 4);
            // compute REMAIN = dp->rec_len - IDEAL_LEN;
            remain = dp->rec_len - ideal_len;

            // if (REMAIN >= NEED_LEN) {
            if (remain >= need_len) { 
               // found enough space for new entry
               // trim dp's rec_len to its ideal length
                dp->rec_len = ideal_len;
                
                cp += dp->rec_len; // advance cp by trimmed length
                dp = (DIR *) cp; // move to next DIR entry
                
                // enter new_entry as [myino, REMIN, strlen(myname), myname]
                dp->inode = myino;
                dp->rec_len = remain;
                dp->name_len = strlen(myname);
                strncpy(dp->name, myname, dp->name_len);
                
                // Write parent's data block back to disk
                put_block(pip->dev, pip->INODE.i_block[i], buf);
                pip->INODE.i_links_count++; // update parent's links count
                pip->INODE.i_atime = time(0L); // update parent's access time
                pip->modified = 1; // mark parent's INODE as dirty
                
                return 0; // success
            }
            
            cp += dp->rec_len; // advance cp by current entry's length
            dp = (DIR *) cp; // move to next DIR entry
        }
    }
    
    return -1; // failure: no space in parent's data blocks
}

int make_dir(char *pathname)
{
    char *parent_pathname, *child_name;
    char temp_pathname[128];
    MINODE *pip;
    int parent, child;
    
    // Extract parent directory pathname and child name

    // WARNING: strtok(), dirname(), basename() destroy pathname
    // so duplicate the pathname before 
    strcpy(temp_pathname, pathname);
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
    child = search(pip, child_name);
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/******************************************************CREATE DIR**********************************************************/

int my_creat(MINODE *pip, char *name)
{
  int ino;
  char buf[BLKSIZE], temp[256];
  DIR* dp;

  // 1. pip points at the parent minode[] of "/a/b", name is a string "c"
  /*
  char *path = "/a/b";
  char *parent_path = dirname(path); // parent_path is now "/a"

  // Find the inode of the parent directory "/a"
  MINODE *parent_mip = path2inode(parent_path);
  if (parent_mip == NULL) {
      printf("Error: directory %s does not exist\n", parent_path);
      return -1;
  }
  */
  /*
  // Get the MINODE structure for the parent inode
  pip = iget(root->dev, pip->ino);
  */
 
  //  2. allocate an inode for the new file
  ino = ialloc(dev);   // Allocate a new inode on the device

  // 3. MINODE *mip = iget(dev, ino); o
  //load inode into a minode[] (in order t0 write contents to the INODE in memory.
  MINODE *mip = iget(dev,ino); 
  INODE *ip = &mip->INODE;

  // 4. Write contents to mip->INODE to make it a REG INODE. Mark it modified;

  ip->i_mode = 0x81A4;		// OR 0100644: REG type and permissions
  ip->i_uid  = running->uid;	// Owner uid 
  ip->i_gid  = running->gid;	// Group Id
  ip->i_size = 0;		// Size in bytes 
  ip->i_links_count = 1;	        // Links count=1 because it's a file
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  // No data blocks allocated yet
  for(int i = 0; i < 15; i++)
  {
    ip->i_block[i] = 0;
  }
 
  mip->modified = 1;            // mark minode MODIFIED
  // 5. iput(mip); which writes the new INODE out to disk.
  iput(mip);

  // 6. Enter name ENTRY into parent's directory by 
  // enter_child(pip, ino, name);
  enter_child(pip, ino, name);
  
  return 0;
}


int creat_file(char *pathname)
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
    child = search(pip, child_name);
    if (child) {
      printf("Error: A file or directory with the same name already exists in the parent directory.\n");
      iput(pip);
      return -1;
    }
    
    // Create new file using my_creat() function
    parent = pip->ino;
    my_creat(pip, child_name);
    
    // Update file inode metadata
    child = search(pip, child_name);
    MINODE *mip = iget(pip->dev, child);
    mip->INODE.i_mode = 0x81A4;
    mip->INODE.i_size = 0;
    mip->INODE.i_links_count = 1;
    mip->INODE.i_atime = time(0L);
    mip->INODE.i_ctime = time(0L);
    mip->INODE.i_mtime = time(0L);
    mip->INODE.i_uid = running->uid;
    mip->INODE.i_gid = running->gid;
    mip->modified = 1;
    iput(mip);
    
    // Release parent directory inode
    iput(pip);
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



